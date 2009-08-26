/*
 *  xfce4-volumed - Volume management daemon for XFCE 4
 *
 *  Copyright © 2009 Steve Dodier <sidnioulz@gmail.com>
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

	/* Get long name of the mixer element */
	factory = gst_element_get_factory (GST_ELEMENT (tmp_mixer));
	long_name = gst_element_factory_get_longname (factory);

	/* Get the device name of the mixer element */
	g_object_get (tmp_mixer, "device-name", &device_name, NULL);
	
	/* Fall back to default name if neccessary */
	if (G_LIKELY (device_name == NULL))
		device_name = g_strdup ("Unknown Volume Control 0");

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
		if (Inst->muted)
			xvd_notify_notification (Inst, "notification-audio-volume-muted", 0);
		else {
			xvd_mixer_init_volume (Inst);
			xvd_notify_volume_notification (Inst);
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
		if (!Inst->muted)
			xvd_notify_volume_notification (Inst);
		g_free (label);
	}
	else if (type == GST_MIXER_MESSAGE_MIXER_CHANGED) {
		// This kind of message shouldn't happen on an hardware card
		#ifndef NDEBUG
		g_print ("GST_MIXER_MESSAGE_MIXER_CHANGED event\n");
		#endif
	}
}
#endif

void 
xvd_mixer_init(XvdInstance *Inst)
{
	/* Get list of all available mixer devices */
	Inst->mixers = gst_audio_default_registry_mixer_filter (_xvd_mixer_filter_mixer, FALSE, Inst);
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
	}
}

void 
xvd_get_xfconf_card_from_mixer(XvdInstance *Inst)
{
	GList      *iter;
	gchar      *xvdgc_card_name;
	
	Inst->card = NULL;

	for (iter = g_list_first (Inst->mixers); iter != NULL; iter = g_list_next (iter)) {
		xvdgc_card_name = g_object_get_data (G_OBJECT (iter->data), "xfce-mixer-internal-name");

		if (G_UNLIKELY (g_utf8_collate (Inst->card_name, xvdgc_card_name) == 0)) {
		  Inst->card = iter->data;
		  break;
		}
	}
	
	if (NULL == Inst->card) {
		if (Inst->card_name) {
			xvd_clean_card_name (Inst);
			g_warning ("The card set in xfconf could not be found.\n");
		}
		iter = g_list_first (Inst->mixers);
		if ((NULL == iter) || (NULL == iter->data)) {
			Inst->card = NULL;
			Inst->card_name = NULL;
			Inst->xvd_init_error = TRUE;
			g_warning ("Gstreamer didn't return any sound card. Init error, going idle.\n");
			return;
		}
		else {
			xvdgc_card_name = g_object_get_data (G_OBJECT (iter->data), "xfce-mixer-internal-name");
			Inst->card_name = g_strdup (xvdgc_card_name);
			Inst->card = iter->data;
			xvd_xfconf_set_card(Inst, Inst->card_name);
			g_warning ("The xfconf property %s has been defaulted to %s.\n", XFCONF_MIXER_ACTIVECARD, Inst->card_name);
		}
	}
	
	#ifdef HAVE_LIBNOTIFY
	gst_element_set_bus (Inst->card, Inst->bus);
	#endif
}

void 
xvd_get_xfconf_track_from_mixer(XvdInstance *Inst, 
								const gchar *xfconf_val)
{
	const GList   *iter;
	gchar         *tmp_label = NULL, *master_label = NULL, *first_label = NULL;
	GstMixerTrack *master_track = NULL, *first_track = NULL;
	
	if (Inst->card_name) {
		for (iter = gst_mixer_list_tracks (GST_MIXER (Inst->card)); iter != NULL; iter = g_list_next (iter)) {
			g_object_get (GST_MIXER_TRACK (iter->data), "label", &tmp_label, NULL);
			
			if (first_label == NULL) {
				first_track = iter->data;
				first_label = g_strdup (tmp_label);
			}
			
			if (master_label == NULL) {
				if (TRUE == GST_MIXER_TRACK_HAS_FLAG ((GstMixerTrack *)iter->data, GST_MIXER_TRACK_MASTER)) {
					master_track = iter->data;
					master_label = g_strdup (tmp_label);
				}
			}
			
			if ((xfconf_val != NULL) && (g_utf8_collate (tmp_label, xfconf_val) == 0)) {
				Inst->track_label = g_strdup (tmp_label);
				Inst->track = iter->data;
				g_free (tmp_label);
				break;
			}
			g_free (tmp_label);
		}
	}
	
	if (NULL == Inst->track_label) {
		g_warning ("There is no xfconf track, trying the first Master track of the card.\n");
		if (NULL == master_label) {
			g_warning ("There is no Master track either, trying the first track of the card.\n");
			if (NULL == first_label) {
				g_warning ("The card doesn't have any track (or there is no card). Xvd init error, going to idle.\n");
				Inst->xvd_init_error = TRUE;
				return;
			}
			else {
				Inst->track_label = g_strdup (first_label);
				Inst->track = first_track;
			}
		}
		else {
			Inst->track_label = g_strdup (master_label);
			Inst->track = master_track;
		}
	}
	
	g_free (first_label);
	g_free (master_label);
	
	if (Inst->xvd_init_error) {
		Inst->xvd_init_error = FALSE;
		#ifndef NDEBUG
		g_print ("The daemon apparently recovered from a card/track initialisation error.\n");
		#endif
	}
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
	g_signal_handler_disconnect (Inst->bus, Inst->bus_id);

	gst_bus_remove_signal_watch (Inst->bus);
	gst_object_unref (Inst->bus);
}
#endif

void 
xvd_clean_track(XvdInstance *Inst)
{
	g_free (Inst->track_label);
/*	Inst->track_label = NULL;*/
}


void 
xvd_calculate_avg_volume(XvdInstance *Inst, 
						 gint *volumes, 
						 gint num_channels)
{
	if (Inst->track) {
		gint i, s=0;
		for (i=0; i<num_channels; i++) {
			s += ((volumes[i] - Inst->track->min_volume) * 100 / 
			     (Inst->track->max_volume - Inst->track->min_volume));
		}
		Inst->current_vol = s/num_channels;
	}
	else {
		Inst->current_vol = 0.0;
	}
}

void 
xvd_mixer_change_volume(XvdInstance *Inst, 
						gint step)
{
	if ((Inst->card_name) && (Inst->track)) {
		gint i;
		gint *volumes = g_malloc (sizeof (gint) * Inst->track->num_channels);
		
		gst_mixer_get_volume (GST_MIXER (Inst->card), Inst->track, volumes);
		
		for (i=0; i<Inst->track->num_channels; i++) {
			volumes[i] += (gint)( ((gdouble)step / 100) * (Inst->track->max_volume - Inst->track->min_volume));
			
			if (volumes[i] > Inst->track->max_volume)
				volumes[i] = Inst->track->max_volume;
				
			if (volumes[i] < Inst->track->min_volume)
				volumes[i] = Inst->track->min_volume;
		}
		
		xvd_calculate_avg_volume (Inst, volumes, Inst->track->num_channels);
		
		gst_mixer_set_volume (GST_MIXER (Inst->card), Inst->track, volumes);
		g_free (volumes);
	}
}

void 
xvd_mixer_toggle_mute(XvdInstance *Inst)
{
	if ((Inst->card_name) && (Inst->track)) {
		gst_mixer_set_mute (GST_MIXER (Inst->card), Inst->track, !(GST_MIXER_TRACK_HAS_FLAG (Inst->track, GST_MIXER_TRACK_MUTE)));
		Inst->muted = (GST_MIXER_TRACK_HAS_FLAG (Inst->track, GST_MIXER_TRACK_MUTE));
	}
}
