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
	xvd_clean_card_name(Inst);
	Inst->card_name = xvd_get_xfconf_card (Inst);
	if (NULL == Inst->card_name) {
		g_warning ("The new active card defined in the xfce mixer seems to be wrong.\n");
		Inst->xvd_init_error = TRUE;
	}
	else {
		
		xvd_get_xfconf_card_from_mixer (Inst);
		#ifndef NDEBUG
		g_print ("New card : %s \n", Inst->card_name);
		#endif
	}
}

static void
_xvd_xfconf_reinit_track(XvdInstance *Inst)
{
	xvd_clean_track (Inst);
	
	gchar *tmp_track = xvd_get_xfconf_track (Inst);
	xvd_get_xfconf_track_from_mixer (Inst, tmp_track);
	g_free (tmp_track);

	xvd_mixer_init_volume (Inst);
	#ifndef NDEBUG
	g_print ("New track : %s with volume %d\n", Inst->track_label, Inst->current_vol);
	#endif
}


static void
_xvd_xfconf_reinit_vol_step(XvdInstance *Inst)
{
		xvd_load_xfconf_vol_step (Inst);
		#ifndef NDEBUG
		g_print ("New volume step : %u\n", Inst->vol_step);
		#endif
}

static void 
_xvd_xfconf_handle_changes(XfconfChannel  *re_channel,
						   const gchar    *re_property_name,
						   const GValue   *re_value,
						   gpointer  	  *ptr)
{
	XvdInstance *Inst = (XvdInstance *)ptr;
	#ifndef NDEBUG
	g_print ("Xfconf event on %s\n", re_property_name);
	#endif
	
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

gchar *
xvd_get_xfconf_card(XvdInstance *Inst)
{
	if (FALSE == xfconf_channel_has_property (Inst->chan, XFCONF_MIXER_ACTIVECARD)) {
		// Transition purpose - we dont watch changes on the legacy property afterwards
		if (FALSE == xfconf_channel_has_property (Inst->chan, XFCONF_MIXER_ACTIVECARD_LEGACY)) {
			g_warning ("%s\n", "Error while trying to retrieve the mixer channel's active card");
			return NULL;
		}
		else {
			g_warning ("%s\n", "Using the legacy xfconf property for the active card");
			gchar *legacy_value = xfconf_channel_get_string (Inst->chan, XFCONF_MIXER_ACTIVECARD_LEGACY, NULL);
			xvd_xfconf_set_card (Inst, legacy_value);
			return legacy_value;
		}
	}
	
	return xfconf_channel_get_string (Inst->chan, XFCONF_MIXER_ACTIVECARD, NULL);
}

gchar *
xvd_get_xfconf_track(XvdInstance *Inst)
{
	return xfconf_channel_get_string (Inst->chan, XFCONF_MIXER_ACTIVETRACK, NULL);
}

void 
xvd_load_xfconf_vol_step(XvdInstance *Inst)
{
	Inst->vol_step = xfconf_channel_get_uint (Inst->chan, XFCONF_MIXER_VOL_STEP, -1);
	if ((Inst->vol_step < 0) || (Inst->vol_step > 100)) {
		Inst->vol_step = VOL_STEP_DEFAULT_VAL;
		xfconf_channel_set_uint (Inst->chan, XFCONF_MIXER_VOL_STEP, VOL_STEP_DEFAULT_VAL);
	}
}

void 
xvd_xfconf_shutdown(XvdInstance *Inst)
{
	xfconf_shutdown ();
}

void 
xvd_xfconf_set_card(XvdInstance *Inst, gchar *value)
{
	xfconf_channel_set_string (Inst->chan, XFCONF_MIXER_ACTIVECARD, value);
}
