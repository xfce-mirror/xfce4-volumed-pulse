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

#include "xvd_keys.h"
#include "xvd_mixer.h"
#ifdef HAVE_LIBNOTIFY
#include "xvd_notify.h"
#endif

static
void xvd_raise_handler (const char *keystring, void *Inst)
{
  XvdInstance *xvd_inst = (XvdInstance *) Inst;
  
  g_debug ("The RaiseVolume key was pressed.");
  
  if (xvd_mixer_change_volume (xvd_inst, xvd_inst->vol_step)) {
		#ifdef HAVE_LIBNOTIFY
			if (xvd_inst->current_vol == 100)
				xvd_notify_overshoot_notification (xvd_inst);
			else
				xvd_notify_volume_notification (xvd_inst);
		#endif
  }
}

static
void xvd_lower_handler (const char *keystring, void *Inst)
{
  XvdInstance *xvd_inst = (XvdInstance *) Inst;
  
  g_debug ("The LowerVolume key was pressed.");
  
  if (xvd_mixer_change_volume (xvd_inst, xvd_inst->vol_step * -1)) {
		#ifdef HAVE_LIBNOTIFY
			if (xvd_inst->current_vol == 0)
				xvd_notify_undershoot_notification (xvd_inst);
			else
				xvd_notify_volume_notification (xvd_inst);
		#endif
  }
}

static
void xvd_mute_handler (const char *keystring, void *Inst)
{
  XvdInstance *xvd_inst = (XvdInstance *) Inst;
  
  g_debug ("The LowerVolume key was pressed.");
  
  if (xvd_mixer_toggle_mute (xvd_inst)) {
		#ifdef HAVE_LIBNOTIFY
			if (xvd_inst->muted)
				xvd_notify_notification (xvd_inst, "audio-volume-muted", xvd_inst->current_vol);
			else {
				xvd_mixer_init_volume (xvd_inst);
				xvd_notify_volume_notification (xvd_inst);
			}
		#endif
	}
}

void
xvd_keys_init(XvdInstance *Inst)
{
    keybinder_init();

    keybinder_bind ("XF86AudioRaiseVolume", xvd_raise_handler, Inst);
    keybinder_bind ("XF86AudioLowerVolume", xvd_lower_handler, Inst);
    keybinder_bind ("XF86AudioMute", xvd_mute_handler, Inst);
}

void
xvd_keys_release (XvdInstance *Inst)
{
    keybinder_unbind ("XF86AudioRaiseVolume", xvd_raise_handler);
    keybinder_unbind ("XF86AudioLowerVolume", xvd_lower_handler);
    keybinder_unbind ("XF86AudioMute", xvd_mute_handler);
}
