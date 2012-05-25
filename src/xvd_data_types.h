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

#ifndef _XVD_DATA_TYPES_H
#define _XVD_DATA_TYPES_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <glib.h>

#include <xfconf/xfconf.h>

#include <pulse/glib-mainloop.h>
#include <pulse/context.h>
#include <pulse/volume.h>

#ifdef HAVE_LIBNOTIFY
#include <libnotify/notification.h>
#endif

#define XFCONF_MIXER_CHANNEL_NAME "xfce4-mixer"
#define XFCONF_MIXER_VOL_STEP "/volume-step-size"
#define VOL_STEP_DEFAULT_VAL 5

#define XVD_APPNAME "Xfce volume daemon"


typedef enum _XvdVolStepDirection
{
  XVD_UP,
  XVD_DOWN
} XvdVolStepDirection;

typedef struct {
	/* PA data */
	pa_glib_mainloop *pa_main_loop;
	pa_context       *pulse_context;
	guint32           sink_index;
	pa_cvolume        volume;
	int               mute;
	
	/* Xfconf vars */
	XfconfChannel		*chan;
	guint				vol_step;
  
  #ifdef HAVE_LIBNOTIFY
    /* Libnotify vars */
	gboolean			gauge_notifications;
	NotifyNotification* notification;
	#endif

	/* Other Xvd vars */
	GMainLoop			*loop;
} XvdInstance;

#endif
