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
	
	if (g_strcmp0 (re_property_name, XFCONF_MIXER_VOL_STEP) == 0) {
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

	Inst->chan = xfconf_channel_get (XFCONF_MIXER_CHANNEL_NAME);
	g_signal_connect (G_OBJECT (Inst->chan), "property-changed", G_CALLBACK (_xvd_xfconf_handle_changes), Inst);
	return TRUE;
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
