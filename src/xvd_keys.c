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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <keybinder.h>

#include "xvd_keys.h"
#include "xvd_pulse.h"


static
void xvd_raise_handler (const char *keystring, void *Inst)
{
  XvdInstance *xvd_inst = (XvdInstance *) Inst;
  
  g_debug ("The RaiseVolume key was pressed.");
  
  xvd_update_volume (xvd_inst,
                     XVD_UP);
}

static
void xvd_lower_handler (const char *keystring, void *Inst)
{
  XvdInstance *xvd_inst = (XvdInstance *) Inst;
  
  g_debug ("The LowerVolume key was pressed.");
  
  xvd_update_volume (xvd_inst,
                     XVD_DOWN);
}

static
void xvd_mute_handler (const char *keystring, void *Inst)
{
  XvdInstance *xvd_inst = (XvdInstance *) Inst;
  
  g_debug ("The LowerVolume key was pressed.");
  
  xvd_toggle_mute (xvd_inst);
}

void
xvd_keys_init(XvdInstance *Inst)
{
    keybinder_init();

    keybinder_bind ("XF86AudioRaiseVolume", xvd_raise_handler, Inst);
    keybinder_bind ("<Ctrl>XF86AudioRaiseVolume", xvd_raise_handler, Inst);
    keybinder_bind ("<Alt>XF86AudioRaiseVolume", xvd_raise_handler, Inst);
    keybinder_bind ("<Super>XF86AudioRaiseVolume", xvd_raise_handler, Inst);
    keybinder_bind ("<Shift>XF86AudioRaiseVolume", xvd_raise_handler, Inst);
    keybinder_bind ("<Ctrl><Shift>XF86AudioRaiseVolume", xvd_raise_handler, Inst);
    keybinder_bind ("<Ctrl><Alt>XF86AudioRaiseVolume", xvd_raise_handler, Inst);
    keybinder_bind ("<Ctrl><Super>XF86AudioRaiseVolume", xvd_raise_handler, Inst);
    keybinder_bind ("<Alt><Shift>XF86AudioRaiseVolume", xvd_raise_handler, Inst);
    keybinder_bind ("<Alt><Super>XF86AudioRaiseVolume", xvd_raise_handler, Inst);
    keybinder_bind ("<Shift><Super>XF86AudioRaiseVolume", xvd_raise_handler, Inst);
    keybinder_bind ("<Ctrl><Shift><Super>XF86AudioRaiseVolume", xvd_raise_handler, Inst);
    keybinder_bind ("<Ctrl><Shift><Alt>XF86AudioRaiseVolume", xvd_raise_handler, Inst);
    keybinder_bind ("<Ctrl><Alt><Super>XF86AudioRaiseVolume", xvd_raise_handler, Inst);
    keybinder_bind ("<Shift><Alt><Super>XF86AudioRaiseVolume", xvd_raise_handler, Inst);
    keybinder_bind ("<Ctrl><Shift><Alt><Super>XF86AudioRaiseVolume", xvd_raise_handler, Inst);
    
    
    keybinder_bind ("XF86AudioLowerVolume", xvd_lower_handler, Inst);
    keybinder_bind ("<Ctrl>XF86AudioLowerVolume", xvd_lower_handler, Inst);
    keybinder_bind ("<Alt>XF86AudioLowerVolume", xvd_lower_handler, Inst);
    keybinder_bind ("<Super>XF86AudioLowerVolume", xvd_lower_handler, Inst);
    keybinder_bind ("<Shift>XF86AudioLowerVolume", xvd_lower_handler, Inst);
    keybinder_bind ("<Ctrl><Shift>XF86AudioLowerVolume", xvd_lower_handler, Inst);
    keybinder_bind ("<Ctrl><Alt>XF86AudioLowerVolume", xvd_lower_handler, Inst);
    keybinder_bind ("<Ctrl><Super>XF86AudioLowerVolume", xvd_lower_handler, Inst);
    keybinder_bind ("<Alt><Shift>XF86AudioLowerVolume", xvd_lower_handler, Inst);
    keybinder_bind ("<Alt><Super>XF86AudioLowerVolume", xvd_lower_handler, Inst);
    keybinder_bind ("<Shift><Super>XF86AudioLowerVolume", xvd_lower_handler, Inst);
    keybinder_bind ("<Ctrl><Shift><Super>XF86AudioLowerVolume", xvd_lower_handler, Inst);
    keybinder_bind ("<Ctrl><Shift><Alt>XF86AudioLowerVolume", xvd_lower_handler, Inst);
    keybinder_bind ("<Ctrl><Alt><Super>XF86AudioLowerVolume", xvd_lower_handler, Inst);
    keybinder_bind ("<Shift><Alt><Super>XF86AudioLowerVolume", xvd_lower_handler, Inst);
    keybinder_bind ("<Ctrl><Shift><Alt><Super>XF86AudioLowerVolume", xvd_lower_handler, Inst);
    
    
    keybinder_bind ("XF86AudioMute", xvd_mute_handler, Inst);
    keybinder_bind ("<Ctrl>XF86AudioMute", xvd_mute_handler, Inst);
    keybinder_bind ("<Alt>XF86AudioMute", xvd_mute_handler, Inst);
    keybinder_bind ("<Super>XF86AudioMute", xvd_mute_handler, Inst);
    keybinder_bind ("<Shift>XF86AudioMute", xvd_mute_handler, Inst);
    keybinder_bind ("<Ctrl><Shift>XF86AudioMute", xvd_mute_handler, Inst);
    keybinder_bind ("<Ctrl><Alt>XF86AudioMute", xvd_mute_handler, Inst);
    keybinder_bind ("<Ctrl><Super>XF86AudioMute", xvd_mute_handler, Inst);
    keybinder_bind ("<Alt><Shift>XF86AudioMute", xvd_mute_handler, Inst);
    keybinder_bind ("<Alt><Super>XF86AudioMute", xvd_mute_handler, Inst);
    keybinder_bind ("<Shift><Super>XF86AudioMute", xvd_mute_handler, Inst);
    keybinder_bind ("<Ctrl><Shift><Super>XF86AudioMute", xvd_mute_handler, Inst);
    keybinder_bind ("<Ctrl><Shift><Alt>XF86AudioMute", xvd_mute_handler, Inst);
    keybinder_bind ("<Ctrl><Alt><Super>XF86AudioMute", xvd_mute_handler, Inst);
    keybinder_bind ("<Shift><Alt><Super>XF86AudioMute", xvd_mute_handler, Inst);
    keybinder_bind ("<Ctrl><Shift><Alt><Super>XF86AudioMute", xvd_mute_handler, Inst);
}

void
xvd_keys_release (XvdInstance *Inst)
{

    keybinder_unbind ("XF86AudioRaiseVolume", xvd_raise_handler);
    keybinder_unbind ("<Ctrl>XF86AudioRaiseVolume", xvd_raise_handler);
    keybinder_unbind ("<Alt>XF86AudioRaiseVolume", xvd_raise_handler);
    keybinder_unbind ("<Super>XF86AudioRaiseVolume", xvd_raise_handler);
    keybinder_unbind ("<Shift>XF86AudioRaiseVolume", xvd_raise_handler);
    keybinder_unbind ("<Ctrl><Shift>XF86AudioRaiseVolume", xvd_raise_handler);
    keybinder_unbind ("<Ctrl><Alt>XF86AudioRaiseVolume", xvd_raise_handler);
    keybinder_unbind ("<Ctrl><Super>XF86AudioRaiseVolume", xvd_raise_handler);
    keybinder_unbind ("<Alt><Shift>XF86AudioRaiseVolume", xvd_raise_handler);
    keybinder_unbind ("<Alt><Super>XF86AudioRaiseVolume", xvd_raise_handler);
    keybinder_unbind ("<Shift><Super>XF86AudioRaiseVolume", xvd_raise_handler);
    keybinder_unbind ("<Ctrl><Shift><Super>XF86AudioRaiseVolume", xvd_raise_handler);
    keybinder_unbind ("<Ctrl><Shift><Alt>XF86AudioRaiseVolume", xvd_raise_handler);
    keybinder_unbind ("<Ctrl><Alt><Super>XF86AudioRaiseVolume", xvd_raise_handler);
    keybinder_unbind ("<Shift><Alt><Super>XF86AudioRaiseVolume", xvd_raise_handler);
    keybinder_unbind ("<Ctrl><Shift><Alt><Super>XF86AudioRaiseVolume", xvd_raise_handler);
    
    
    keybinder_unbind ("XF86AudioLowerVolume", xvd_lower_handler);
    keybinder_unbind ("<Ctrl>XF86AudioLowerVolume", xvd_lower_handler);
    keybinder_unbind ("<Alt>XF86AudioLowerVolume", xvd_lower_handler);
    keybinder_unbind ("<Super>XF86AudioLowerVolume", xvd_lower_handler);
    keybinder_unbind ("<Shift>XF86AudioLowerVolume", xvd_lower_handler);
    keybinder_unbind ("<Ctrl><Shift>XF86AudioLowerVolume", xvd_lower_handler);
    keybinder_unbind ("<Ctrl><Alt>XF86AudioLowerVolume", xvd_lower_handler);
    keybinder_unbind ("<Ctrl><Super>XF86AudioLowerVolume", xvd_lower_handler);
    keybinder_unbind ("<Alt><Shift>XF86AudioLowerVolume", xvd_lower_handler);
    keybinder_unbind ("<Alt><Super>XF86AudioLowerVolume", xvd_lower_handler);
    keybinder_unbind ("<Shift><Super>XF86AudioLowerVolume", xvd_lower_handler);
    keybinder_unbind ("<Ctrl><Shift><Super>XF86AudioLowerVolume", xvd_lower_handler);
    keybinder_unbind ("<Ctrl><Shift><Alt>XF86AudioLowerVolume", xvd_lower_handler);
    keybinder_unbind ("<Ctrl><Alt><Super>XF86AudioLowerVolume", xvd_lower_handler);
    keybinder_unbind ("<Shift><Alt><Super>XF86AudioLowerVolume", xvd_lower_handler);
    keybinder_unbind ("<Ctrl><Shift><Alt><Super>XF86AudioLowerVolume", xvd_lower_handler);
    
    
    keybinder_unbind ("XF86AudioMute", xvd_mute_handler);
    keybinder_unbind ("<Ctrl>XF86AudioMute", xvd_mute_handler);
    keybinder_unbind ("<Alt>XF86AudioMute", xvd_mute_handler);
    keybinder_unbind ("<Super>XF86AudioMute", xvd_mute_handler);
    keybinder_unbind ("<Shift>XF86AudioMute", xvd_mute_handler);
    keybinder_unbind ("<Ctrl><Shift>XF86AudioMute", xvd_mute_handler);
    keybinder_unbind ("<Ctrl><Alt>XF86AudioMute", xvd_mute_handler);
    keybinder_unbind ("<Ctrl><Super>XF86AudioMute", xvd_mute_handler);
    keybinder_unbind ("<Alt><Shift>XF86AudioMute", xvd_mute_handler);
    keybinder_unbind ("<Alt><Super>XF86AudioMute", xvd_mute_handler);
    keybinder_unbind ("<Shift><Super>XF86AudioMute", xvd_mute_handler);
    keybinder_unbind ("<Ctrl><Shift><Super>XF86AudioMute", xvd_mute_handler);
    keybinder_unbind ("<Ctrl><Shift><Alt>XF86AudioMute", xvd_mute_handler);
    keybinder_unbind ("<Ctrl><Alt><Super>XF86AudioMute", xvd_mute_handler);
    keybinder_unbind ("<Shift><Alt><Super>XF86AudioMute", xvd_mute_handler);
    keybinder_unbind ("<Ctrl><Shift><Alt><Super>XF86AudioMute", xvd_mute_handler);

}
