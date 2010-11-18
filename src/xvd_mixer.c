/*
 *  xfce4-volumed - Volume management daemon for XFCE 4
 *
 *  Copyright © 2009
 *		Steve Dodier <sidnioulz@gmail.com>
 *		Jannis Pohlmann <jannis@xfce.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "xvd_data_types.h"
#include "xvd_mixer.h"
#include "xvd_xfconf.h"
#ifdef HAVE_LIBNOTIFY
#include "xvd_notify.h"
#endif

static gboolean 
_xvd_mixer_filter_mixer (GstMixer *tmp_mixer, 
						 gpointer user_data)
{
	GstElementFactory *factory;
	const gchar       *long_name;
	gchar             *device_name;
	gchar             *internal_name;
	gchar             *name;
	gchar             *p;
	gint               length;
	gint              *counter = user_data;

	/* Get long name of the mixer element */
	factory = gst_element_get_factory (GST_ELEMENT (tmp_mixer));
	long_name = gst_element_factory_get_longname (factory);

	/* Get the device name of the mixer element */
	if (g_object_class_find_property (G_OBJECT_GET_CLASS (G_OBJECT (tmp_mixer)), "device-name"))
		g_object_get (tmp_mixer, "device-name", &device_name, NULL);
	
	/* Fall back to default name if neccessary */
	if (G_UNLIKELY (device_name == NULL))
		device_name = g_strdup_printf ("Unknown Volume Control %d", (*counter)++);

	/* Build display name */
	name = g_strdup_printf ("%s (%s)", device_name, long_name);

	/* Free device name */
	g_free (device_name);
	
	/* Count alpha-numeric characters in the name */
	for (length = 0, p = name; *p != '\0'; ++p)
		if (g_ascii_isalnum (*p))
			++length;

	/* Generate internal name */
	internal_name = g_new0 (gchar, length+1);
	for (length = 0, p = name; *p != '\0'; ++p)
	if (g_ascii_isalnum (*p))
	internal_name[length++] = *p;
	internal_name[length] = '\0';

	/* Remember name for use by xfce4-mixer */
	g_object_set_data_full (G_OBJECT (tmp_mixer), "xfce-mixer-internal-name", internal_name, (GDestroyNotify) g_free);
			
	g_free (name);
	
	return TRUE;
}

#ifdef HAVE_LIBNOTIFY
static void 
_xvd_mixer_bus_message (GstBus *bus, GstMessage *message, 
						gpointer data)
{
	GstMixerMessageType type;
	GstMixerTrack      *msg_track = NULL;
	gchar              *label;
	gint               *volumes;
	gint                num_channels;
	XvdInstance        *Inst = data;

	if (G_UNLIKELY (GST_MESSAGE_SRC (message) != GST_OBJECT (Inst->card)))
		return;

	type = gst_mixer_message_get_type (message);

	if (type == GST_MIXER_MESSAGE_MUTE_TOGGLED)
	{
		gst_mixer_message_parse_mute_toggled (message, &msg_track, &Inst->muted);
		g_object_get (msg_track, "label", &label, NULL);
		if (g_strcmp0 (Inst->track_label, label) != 0)
			return;
#ifdef HAVE_LIBNOTIFY
		if (Inst->muted)
			xvd_notify_notification (Inst, "audio-volume-muted", 0);
		else {
			xvd_mixer_init_volume (Inst);
			xvd_notify_volume_notification (Inst);
#endif
		}
		g_free (label);
	}
	else if (type == GST_MIXER_MESSAGE_VOLUME_CHANGED)
	{
		gst_mixer_message_parse_volume_changed (message, &msg_track, &volumes, &num_channels);
		g_object_get (msg_track, "label", &label, NULL);
		if (g_strcmp0 (Inst->track_label, label) != 0)
			return;
		xvd_calculate_avg_volume (Inst, volumes, num_channels);
#ifdef HAVE_LIBNOTIFY
		xvd_notify_volume_notification (Inst);
#endif
		g_free (label);
	}
	else if (type == GST_MIXER_MESSAGE_MIXER_CHANGED) {
		// This kind of message shouldn't happen on an hardware card
		g_debug ("GST_MIXER_MESSAGE_MIXER_CHANGED event\n");
	}
}
#endif

void 
xvd_mixer_init(XvdInstance *Inst)
{
	/* Get list of all available mixer devices */
	Inst->mixers = gst_audio_default_registry_mixer_filter (_xvd_mixer_filter_mixer, FALSE, &(Inst->nameless_cards_count));
}

#ifdef HAVE_LIBNOTIFY
void 
xvd_mixer_init_bus(XvdInstance *Inst)
{
	/* Create a GstBus for notifications */
	Inst->bus = gst_bus_new ();
	Inst->bus_id = g_signal_connect (Inst->bus, "message::element", G_CALLBACK (_xvd_mixer_bus_message), Inst);
	gst_bus_add_signal_watch (Inst->bus);
}
#endif

void 
xvd_mixer_init_volume(XvdInstance *Inst)
{
	if ((Inst->card) && (Inst->track)) {
		gint *volumes = g_malloc (sizeof (gint) * Inst->track->num_channels);
		gst_mixer_get_volume (GST_MIXER (Inst->card), Inst->track, volumes);
		xvd_calculate_avg_volume (Inst, volumes, Inst->track->num_channels);
		g_free (volumes);
		Inst->muted = (GST_MIXER_TRACK_HAS_FLAG (Inst->track, GST_MIXER_TRACK_MUTE));
	}
}

void 
xvd_get_card_from_mixer(XvdInstance *Inst, 
						const gchar *wanted_card,
						const gchar *preferred_fallback)
{
	GList      *iter;
	gchar      *tmp_card_name = NULL, *first_name = NULL;
	GstElement *fallback_card = NULL, *first_card = NULL;

	// Cleaning the current card	
	Inst->card = NULL;
	xvd_clean_card_name (Inst);
	
	// We try to find the card the user wants
	for (iter = g_list_first (Inst->mixers); iter != NULL; iter = g_list_next (iter)) {
		tmp_card_name = g_object_get_data (G_OBJECT (iter->data), "xfce-mixer-internal-name");
		
		if ((wanted_card != NULL) && (G_UNLIKELY (g_strcmp0 (wanted_card, tmp_card_name) == 0))) {
		  Inst->card = iter->data;
		  Inst->card_name = g_strdup (wanted_card);
		  break;
		}

		// If the fallback card label is set, we save the fallback card in case the wanted one isn't found
		if ((preferred_fallback != NULL) && (G_UNLIKELY (g_strcmp0 (preferred_fallback, tmp_card_name) == 0))) {
		  fallback_card = iter->data;
		}
		
		// If no card is asked by the user, or if the asked card(s) couldn't be found, use the first one available
		if (first_name == NULL) {
			first_card = iter->data;
			first_name = g_strdup (tmp_card_name);
			if ((wanted_card == NULL) && (preferred_fallback == NULL))
				break;
		}
	}
	
	// We now check if the card was set or if we should use fallback / first instead
	if (NULL != Inst->card) {
		g_debug ("The card %s was found and set as the current card.\n", wanted_card);
	}
	else if (NULL != fallback_card) {
		g_debug ("The wanted card could not be found, using the fallback one instead.\n");
		Inst->card_name = g_strdup (preferred_fallback);
		Inst->card = fallback_card;
	}
	else if (NULL != first_card) {
		if (wanted_card != NULL) {
			g_debug ("The wanted card could not be found, using the first one instead.\n");
		}
		else {
			g_debug ("Setting the first card in the xfconf property since there was no card set.\n");
			xvd_xfconf_set_card (Inst, first_name);
		}
		Inst->card_name = g_strdup (first_name);
		Inst->card = first_card;
	}
	else {
		g_debug ("Error: there is no sound card on this machine.\n");
		return;
	}
	
	g_free (first_name);

	#ifdef HAVE_LIBNOTIFY
	gst_element_set_bus (Inst->card, Inst->bus);
	#endif
}

void 
xvd_get_track_from_mixer(XvdInstance *Inst, 
							const gchar *wanted_track,
							const gchar *preferred_fallback)
{
	const GList   *iter;
	gchar         *tmp_label = NULL, *master_label = NULL, *first_label = NULL;
	GstMixerTrack *fallback_track = NULL, *master_track = NULL, *first_track = NULL;
	
	// We clean the current track before setting another one
	xvd_clean_track (Inst);
	Inst->track = NULL;
	
	// We're going to go through the available tracks
	if (Inst->card) {
		for (iter = gst_mixer_list_tracks (GST_MIXER (Inst->card)); iter != NULL; iter = g_list_next (iter)) {
			g_object_get (GST_MIXER_TRACK (iter->data), "label", &tmp_label, NULL);
			
			// If the wanted track is requested and found
			if ((wanted_track != NULL) && (g_strcmp0 (tmp_label, wanted_track) == 0)) {
				Inst->track_label = g_strdup (tmp_label);
				Inst->track = iter->data;
				g_free (tmp_label);
				break;
			}
			
			// If the fallback track label is set, we save the fallback track in case the wanted one isn't found
			if ((preferred_fallback != NULL) && (g_strcmp0 (tmp_label, preferred_fallback) == 0)) {
				fallback_track = iter->data;
			}
			
			// If we spot a Master track in the card, we save it in case the xfconf / fallback ones can't be found
			if ((master_label == NULL) && (TRUE == GST_MIXER_TRACK_HAS_FLAG ((GstMixerTrack *)iter->data, GST_MIXER_TRACK_MASTER))) {
				master_track = iter->data;
				master_label = g_strdup (tmp_label);
			}
			
			// We save the first track of the card in case there is no xfconf / fallback / master track
			if (first_label == NULL) {
				first_track = iter->data;
				first_label = g_strdup (tmp_label);
			}
			
			g_free (tmp_label);
		}
	}
	else {
		g_debug ("Error: there is no sound card to search tracks from.\n");
		return;
	}
	
	if (NULL != Inst->track) {
		g_debug ("The track %s was found on the card and set as the current track.\n", wanted_track);
	}
	else if (NULL != fallback_track) {
		g_debug ("The wanted track could not be found, using the fallback one instead.\n");
		Inst->track_label = g_strdup (preferred_fallback);
		Inst->track = fallback_track;
	}
	else if (NULL != master_track) {
		g_debug ("The wanted track could not be found, using the first Master one instead.\n");
		Inst->track_label = g_strdup (master_label);
		Inst->track = master_track;
	}
	else if (NULL != first_track) {
		g_debug ("The wanted track could not be found, using the first one instead.\n");
		Inst->track_label = g_strdup (first_label);
		Inst->track = first_track;
	}
	else {
		g_debug ("Error: the current sound card doesn't have any track.\n");
		return;
	}
	
	g_free (first_label);
	g_free (master_label);
}

static void 
_xvd_mixer_destroy_mixer (GstMixer *mixer)
{
	gst_element_set_state (GST_ELEMENT (mixer), GST_STATE_NULL);
	gst_object_unref (GST_OBJECT (mixer));
}

void 
xvd_clean_card_name(XvdInstance *Inst)
{
	g_free (Inst->card_name);
}

void 
xvd_clean_cards(XvdInstance *Inst)
{
	g_list_foreach (Inst->mixers, (GFunc) _xvd_mixer_destroy_mixer, NULL);
	g_list_free (Inst->mixers);
}

#ifdef HAVE_LIBNOTIFY
void
xvd_clean_mixer_bus(XvdInstance *Inst)
{
	int i, refc = GST_OBJECT_REFCOUNT(Inst->bus);
	gst_bus_remove_signal_watch (Inst->bus);
	g_signal_handler_disconnect (Inst->bus, Inst->bus_id);
	
	for (i=0; i < refc; i++)
		gst_object_unref (Inst->bus);
}
#endif

void 
xvd_clean_track(XvdInstance *Inst)
{
	g_free (Inst->track_label);
}


void 
xvd_calculate_avg_volume(XvdInstance *Inst, 
						 gint *volumes, 
						 gint num_channels)
{
	if (Inst->track) {
		gint i, s=0, step;
		for (i=0; i<num_channels; i++) {
			step=Inst->track->max_volume - Inst->track->min_volume;
			if (!step)
				++step;
			
			s += ((volumes[i] - Inst->track->min_volume) * 100 / step);
		}
		Inst->current_vol = s/num_channels;
	}
	else {
		Inst->current_vol = 0.0;
	}
}

static gint
_nearest_int (gdouble val)
{
	gdouble diff = (val - (gint) val);

	if ((-0.5 < diff) && (diff < 0.5))
		return (gint) val;
	else if (diff > 0)
		return (gint) val + 1;
	else
		return (gint) val - 1;
}

gboolean 
xvd_mixer_change_volume(XvdInstance *Inst, 
						gint step)
{
	if ((Inst->card) && (Inst->track)) {
		gint i;
		gint *volumes = g_malloc (sizeof (gint) * Inst->track->num_channels);
		
		gst_mixer_get_volume (GST_MIXER (Inst->card), Inst->track, volumes);
		
		for (i=0; i<Inst->track->num_channels; i++) {
			volumes[i] += _nearest_int ((gdouble)(step * (Inst->track->max_volume - Inst->track->min_volume)) / 100.0);
			
			if (volumes[i] > Inst->track->max_volume)
				volumes[i] = Inst->track->max_volume;
				
			if (volumes[i] < Inst->track->min_volume)
				volumes[i] = Inst->track->min_volume;
		}
		xvd_calculate_avg_volume (Inst, volumes, Inst->track->num_channels);
		
		gst_mixer_set_volume (GST_MIXER (Inst->card), Inst->track, volumes);
		g_free (volumes);
		
		return TRUE;
	}
	return FALSE;
}

gboolean 
xvd_mixer_toggle_mute(XvdInstance *Inst)
{
	if ((Inst->card) && (Inst->track)) {
		gst_mixer_set_mute (GST_MIXER (Inst->card), Inst->track, !(GST_MIXER_TRACK_HAS_FLAG (Inst->track, GST_MIXER_TRACK_MUTE)));
		Inst->muted = (GST_MIXER_TRACK_HAS_FLAG (Inst->track, GST_MIXER_TRACK_MUTE));
		return TRUE;
	}
	return FALSE;
}
