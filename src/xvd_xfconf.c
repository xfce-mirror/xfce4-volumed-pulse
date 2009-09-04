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

#include "xvd_xfconf.h"
#include "xvd_mixer.h"

static void
_xvd_xfconf_reinit_card(XvdInstance *Inst)
{
	gchar *previous_card = NULL;
	
	if (!xvd_xfconf_get_card (Inst)) {
		// If we fail to get an xfconf card, we save the current one in xfconf
		if (Inst->card_name != NULL) {
			xvd_xfconf_set_card (Inst, Inst->card_name);
			Inst->xfconf_card_name = g_strdup (Inst->card_name);
		}
		// If the current card is NULL too, we do nothing
		return;
	}
	
	// If the card set in xfconf is the same as the currently used one, we do nothing
	if ((Inst->card_name != NULL) && (g_strcmp0 (Inst->xfconf_card_name, Inst->card_name) == 0)) {
		return;
	}
	
	// We now try to replace it with the one set in xfconf
	previous_card = g_strdup (Inst->card_name);
	xvd_get_card_from_mixer (Inst, Inst->xfconf_card_name, previous_card);
	
	// At this stage the track grabbed is wrong, but we expect the user to also update the track key
	
	// We check if the card has been correctly set
	if ((Inst->card_name == NULL) || (g_strcmp0 (Inst->xfconf_card_name, Inst->card_name) != 0)) {
		g_debug ("The card chosen in xfconf could not be set, another one was set instead\nChosen: %s\nSet: %s\n",
														Inst->xfconf_card_name,
														Inst->card_name);
		// If not, we save the valid card in xfconf instead of the user chosen
		// TODO we should actually refresh the mixers prior to finding the new card
		xvd_xfconf_set_card (Inst, Inst->card_name);
		g_free (Inst->xfconf_card_name);
		Inst->xfconf_card_name = g_strdup (Inst->card_name);
	}
	else {
		g_debug ("The card change succeeded with the new xfconf card %s.\n", Inst->xfconf_card_name);
	}
	
	g_free (previous_card);
	
	// If an xfconf track has failed to be applied before, it's probably that the user chosed his new track before the card to which it belongs.
	// So we check if the track belongs to our new card.
	if (Inst->previously_set_track_label) {
		xvd_get_track_from_mixer (Inst, Inst->previously_set_track_label, Inst->track_label);
		if (g_strcmp0 (Inst->previously_set_track_label, Inst->track_label) == 0) {
			xvd_xfconf_set_track (Inst, Inst->previously_set_track_label);
			g_free (Inst->previously_set_track_label);
			g_debug ("The previously set xfconf track was a track from the newly set sound card.\n");
		}
	}
	// Else, we can still try to see if the current track applies
	else {
		xvd_get_track_from_mixer (Inst, Inst->track_label, NULL);
	}
	
	xvd_mixer_init_volume (Inst);

	g_debug ("Xfconf reinit: the card is now %s, the track is %s and the volume is %d\n", Inst->card_name, Inst->track_label, Inst->current_vol);
}

static void
_xvd_xfconf_reinit_track(XvdInstance *Inst)
{
	gchar *previous_track = NULL;
	
	if (!xvd_xfconf_get_track (Inst)) {
		// If we fail to get an xfconf track, we save the current one in xfconf
		if (Inst->track_label != NULL) {
			xvd_xfconf_set_track (Inst, Inst->track_label);
			Inst->xfconf_track_label = g_strdup (Inst->track_label);
		}
		// If the current track is NULL too, we do nothing
		return;
	}
	
	// If the track set in xfconf is the same as the currently used one, we do nothing
	if ((Inst->track_label != NULL) && (g_strcmp0 (Inst->xfconf_track_label, Inst->track_label) == 0)) {
		return;
	}
	
	// We now try to replace it with the one set in xfconf
	previous_track = g_strdup (Inst->track_label);
	xvd_get_track_from_mixer (Inst, Inst->xfconf_track_label, previous_track);

	// We check if the track has been correctly set
	if ((Inst->track_label == NULL) || (g_strcmp0 (Inst->xfconf_track_label, Inst->track_label) != 0)) {
		// If not, we save the valid track in xfconf instead of the user chosen
		Inst->previously_set_track_label = g_strdup (Inst->xfconf_track_label);
		g_debug ("The track chosen in xfconf (%s) doesn't exist in the current card. It'll be tried again after a sound card change.\nNow using %s.\n",
					Inst->xfconf_track_label,
					Inst->track_label);
		xvd_xfconf_set_track (Inst, Inst->track_label);
		g_free (Inst->xfconf_track_label);
		Inst->xfconf_track_label = g_strdup (Inst->track_label);
	}
	else {
		g_free (Inst->previously_set_track_label);
	}
	
	xvd_mixer_init_volume (Inst);
	g_free (previous_track);

	g_debug ("Xfconf reinit: the track is now %s and the volume is %d\n", Inst->track_label, Inst->current_vol);
}


static void
_xvd_xfconf_reinit_vol_step(XvdInstance *Inst)
{
		xvd_xfconf_get_vol_step (Inst);
		g_debug ("Xfconf reinit: volume step is now %u\n", Inst->vol_step);
}

static void 
_xvd_xfconf_handle_changes(XfconfChannel  *re_channel,
						   const gchar    *re_property_name,
						   const GValue   *re_value,
						   gpointer  	  *ptr)
{
	XvdInstance *Inst = (XvdInstance *)ptr;
	g_debug ("Xfconf event on %s\n", re_property_name);
	
	if (g_strcmp0 (re_property_name, XFCONF_MIXER_ACTIVECARD) == 0) {
		_xvd_xfconf_reinit_card(Inst);
	}
	else if (g_strcmp0 (re_property_name, XFCONF_MIXER_ACTIVETRACK) == 0) {
		_xvd_xfconf_reinit_track(Inst);
		_xvd_xfconf_reinit_vol_step(Inst);
	}
	else if (g_strcmp0 (re_property_name, XFCONF_MIXER_VOL_STEP) == 0) {
		_xvd_xfconf_reinit_vol_step(Inst);
	}
}

void 
xvd_xfconf_init(XvdInstance *Inst)
{
	if (FALSE == xfconf_init (&Inst->error)) {
		g_warning ("%s%s\n", "Couldn't initialize xfconf : ", Inst->error->message);
        g_error_free (Inst->error);
        exit (EXIT_FAILURE);
	}

	Inst->chan = xfconf_channel_get (XFCONF_MIXER_CHANNEL_NAME);
	g_signal_connect (G_OBJECT (Inst->chan), "property-changed", G_CALLBACK (_xvd_xfconf_handle_changes), Inst);
}

gboolean 
xvd_xfconf_get_card(XvdInstance *Inst)
{
	if (Inst->xfconf_card_name) {
		g_debug ("%s\n", "Cleaning the current card name stored in xfconf");
		g_free (Inst->xfconf_card_name);
	}
	
	if (FALSE == xfconf_channel_has_property (Inst->chan, XFCONF_MIXER_ACTIVECARD)) {
		// Transition purpose - we dont watch changes on the legacy property afterwards
		if (FALSE == xfconf_channel_has_property (Inst->chan, XFCONF_MIXER_ACTIVECARD_LEGACY)) {
			g_debug ("%s\n", "There is no card name stored in xfconf");
			return FALSE;
		}
		else {
			g_debug ("%s\n", "Using the legacy xfconf property for the card name, and saving its value into the new xfconf property");
			Inst->xfconf_card_name = xfconf_channel_get_string (Inst->chan, XFCONF_MIXER_ACTIVECARD_LEGACY, NULL);
			xvd_xfconf_set_card (Inst, Inst->xfconf_card_name);
			g_debug ("%s %s\n", "Xfconf card name:", Inst->xfconf_card_name);
			return Inst->xfconf_card_name != NULL;
		}
	}
	
	Inst->xfconf_card_name = xfconf_channel_get_string (Inst->chan, XFCONF_MIXER_ACTIVECARD, NULL);
	return Inst->xfconf_card_name != NULL;
}

void 
xvd_xfconf_set_card(XvdInstance *Inst, gchar *value)
{
	g_debug ("%s %s\n", "Setting the xfconf card name to", value);
	xfconf_channel_set_string (Inst->chan, XFCONF_MIXER_ACTIVECARD, value);
}

gboolean
xvd_xfconf_get_track(XvdInstance *Inst)
{
	if (Inst->xfconf_track_label) {
		g_debug ("%s\n", "Cleaning the current track label stored in xfconf");
		g_free (Inst->xfconf_track_label);
	}
	
	Inst->xfconf_track_label = xfconf_channel_get_string (Inst->chan, XFCONF_MIXER_ACTIVETRACK, NULL);
	if (Inst->xfconf_track_label != NULL) {
		g_debug ("%s %s\n", "Xfconf track label:", Inst->xfconf_track_label);
		return TRUE;
	}
	else {
		g_debug ("%s\n", "There is no track label stored in xfconf");
		return FALSE;
	}
}

void 
xvd_xfconf_set_track(XvdInstance *Inst, gchar *value)
{
	g_debug("%s %s\n", "Setting the xfconf card name to", value);
	xfconf_channel_set_string (Inst->chan, XFCONF_MIXER_ACTIVETRACK, value);
}

void 
xvd_xfconf_get_vol_step(XvdInstance *Inst)
{
	Inst->vol_step = xfconf_channel_get_uint (Inst->chan, XFCONF_MIXER_VOL_STEP, -1);
	if ((Inst->vol_step < 0) || (Inst->vol_step > 100)) {
		g_debug ("%s\n", "The volume step xfconf property is out of range, setting back to default");
		Inst->vol_step = VOL_STEP_DEFAULT_VAL;
		xfconf_channel_set_uint (Inst->chan, XFCONF_MIXER_VOL_STEP, VOL_STEP_DEFAULT_VAL);
	}
	g_debug("%s %u\n", "Xfconf volume step:", Inst->vol_step);
}

void 
xvd_xfconf_shutdown(XvdInstance *Inst)
{
	xfconf_shutdown ();
}
