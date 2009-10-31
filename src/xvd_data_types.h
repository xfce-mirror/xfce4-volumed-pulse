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

#define XFCONF_MIXER_CHANNEL_NAME "xfce4-mixer"
#define XFCONF_MIXER_ACTIVECARD "/active-card"
#define XFCONF_MIXER_ACTIVECARD_LEGACY "/sound-card"
#define XFCONF_MIXER_ACTIVETRACK "/active-track"
#define XFCONF_MIXER_VOL_STEP "/volume-step-size"
#define VOL_STEP_DEFAULT_VAL 5

#define XVD_APPNAME "Xfce volume daemon"

#include <stdlib.h>
#include <unistd.h>

#include <xfconf/xfconf.h>

#include <gst/audio/mixerutils.h>

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <X11/XF86keysym.h>
#ifdef HAVE_LIBNOTIFY
#include <libnotify/notification.h>
#endif

typedef struct {
	/* Sound card being used and list of cards */
	GList     			*mixers;
	GstElement      	*card;
	gchar				*card_name;
	gint				nameless_cards_count;
	
	/* Tracks for the card */
	GstMixerTrack   	*track;
	gchar           	*track_label;

	/* Xfconf vars */
	GError     			*error;
	XfconfChannel		*chan;
	gchar				*xfconf_card_name;
	gchar           	*xfconf_track_label;
	gchar				*previously_set_track_label;

	/* Gstreamer bus vars */
	GstBus				*bus;
	guint				bus_id;

	/* Volume vars */
	guint				current_vol;
	guint				vol_step;
	gboolean        	muted;
	
	/* Xcb vars */
	xcb_connection_t 	*conn;
	xcb_window_t 		root_win;
	xcb_key_symbols_t 	*kss;
	#ifndef LEGACY_XCBKEYSYMS
	xcb_keycode_t 		*keyRaise,
						*keyLower,
						*keyMute;
	#else
	xcb_keycode_t 		keyRaise,
						keyLower,
						keyMute;
	#endif
    
    #ifdef HAVE_LIBNOTIFY
    /* Libnotify vars */
	gboolean			gauge_notifications;
	NotifyNotification* notification;
	#endif

	/* Other Xvd vars */
	GMainLoop			*loop;
} XvdInstance;

#endif
