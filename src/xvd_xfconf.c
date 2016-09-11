/*
 *  xfce4-volumed - Volume management daemon for XFCE 4
 *
 *  Copyright © 2009 Steve Dodier <sidnioulz@gmail.com>
 *  Copyright © 2012 Lionel Le Folgoc <lionel@lefolgoc.net>
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

	if (g_strcmp0 (re_property_name, XFCONF_MIXER_VOL_STEP_PROP) == 0) {
		_xvd_xfconf_reinit_vol_step(Inst);
	}
}

gboolean
xvd_xfconf_init(XvdInstance *Inst)
{
	GError *err = NULL;

	if (!xfconf_init (&err)) {
		g_warning ("%s%s\n", "Couldn't initialize xfconf: ", err->message);
		g_error_free (err);
		return FALSE;
	}

	/* Initialize an xfconf channel for xfce4-volumed-pulse */
	Inst->settings = xfconf_channel_new (XFCONF_VOLUMED_PULSE_CHANNEL_NAME);
	if (!xfconf_channel_has_property (Inst->settings, XFCONF_ICON_STYLE_PROP)) {
		if (!xfconf_channel_set_uint (Inst->settings, XFCONF_ICON_STYLE_PROP,
									  ICONS_STYLE_NORMAL))
			g_warning ("Couldn't initialize icon-style property (default: 0).");
	}

	if (!xfconf_channel_has_property (Inst->settings, XFCONF_MIXER_VOL_STEP_PROP)) {
		if (!xfconf_channel_set_uint (Inst->settings, XFCONF_MIXER_VOL_STEP_PROP,
									  VOL_STEP_DEFAULT_VAL))
			g_warning ("Couldn't initialize the volume-step-size property (default: 5).");
	}

	g_signal_connect (G_OBJECT (Inst->settings), "property-changed", G_CALLBACK (_xvd_xfconf_handle_changes), Inst);

	return TRUE;
}

void
xvd_xfconf_get_vol_step(XvdInstance *Inst)
{
	Inst->vol_step = xfconf_channel_get_uint (Inst->settings, XFCONF_MIXER_VOL_STEP_PROP, VOL_STEP_DEFAULT_VAL);
	if (Inst->vol_step > 100) {
		g_debug ("%s\n", "The volume step xfconf property is out of range, setting back to default");
		Inst->vol_step = VOL_STEP_DEFAULT_VAL;
		xfconf_channel_set_uint (Inst->settings, XFCONF_MIXER_VOL_STEP_PROP, VOL_STEP_DEFAULT_VAL);
	}
	g_debug("%s %u\n", "Xfconf volume step:", Inst->vol_step);
}

void
xvd_xfconf_shutdown(XvdInstance *Inst)
{
	if(Inst->settings)
		g_object_unref(Inst->settings);
	xfconf_shutdown ();
}
