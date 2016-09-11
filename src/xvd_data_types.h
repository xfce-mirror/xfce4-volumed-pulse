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

#define XFCONF_VOLUMED_PULSE_CHANNEL_NAME "xfce4-volumed-pulse"
#define XFCONF_MIXER_VOL_STEP_PROP "/volume-step-size"
#define VOL_STEP_DEFAULT_VAL 5
#define XFCONF_ICON_STYLE_PROP "/icon-style"
#define ICONS_STYLE_NORMAL 0
#define ICONS_STYLE_SYMBOLIC 1

#define XVD_APPNAME "Xfce volume daemon"

/* Icon names for the various audio notifications */
#define ICON_AUDIO_VOLUME_MUTED		"audio-volume-muted"
#define ICON_AUDIO_VOLUME_OFF		"audio-volume-off"
#define ICON_AUDIO_VOLUME_LOW		"audio-volume-low"
#define ICON_AUDIO_VOLUME_MEDIUM	"audio-volume-medium"
#define ICON_AUDIO_VOLUME_HIGH		"audio-volume-high"
#define ICON_MICROPHONE_MUTED		"microphone-sensitivity-muted"
#define ICON_MICROPHONE_HIGH		"microphone-sensitivity-high"

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
	guint32           source_index;
	pa_cvolume        volume;
	int               mute;
	int               mic_mute;

	/* Xfconf vars */
	XfconfChannel		*chan;
	XfconfChannel       *settings;
	guint				vol_step;

  #ifdef HAVE_LIBNOTIFY
    /* Libnotify vars */
	gboolean			gauge_notifications;
	NotifyNotification* notification;
	NotifyNotification* notification_mic;
	#endif

	/* Other Xvd vars */
	GMainLoop			*loop;
} XvdInstance;

#endif
