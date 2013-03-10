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

#ifndef _XVD_NOTIFY_H
#define _XVD_NOTIFY_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "xvd_data_types.h"


#define SYNCHRONOUS      "x-canonical-private-synchronous"
#define LAYOUT_ICON_ONLY "x-canonical-private-icon-only"


void 
xvd_notify_notification(XvdInstance *Inst, 
						gchar* icon, 
						gint value);

void 
xvd_notify_volume_notification(XvdInstance *Inst);

void
xvd_notify_overshoot_notification(XvdInstance *Inst);

void
xvd_notify_undershoot_notification(XvdInstance *Inst);

void 
xvd_notify_init(XvdInstance *Inst, 
				const gchar *appname);

void 
xvd_notify_uninit(XvdInstance *Inst);

#endif
