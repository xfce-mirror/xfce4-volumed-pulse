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

#ifndef LEGACY_XCBKEYSYMS
// This is only needed when the keycodes are a list and not a struct
static gboolean
_xvd_keys_is_symbol(xcb_keycode_t elem,
					xcb_keycode_t *list)
{
	gint i = 0;
	gboolean belongs = FALSE;
	
	if (list == NULL)
		return FALSE;
	
	while (list[i] != XCB_NO_SYMBOL) {
		belongs |= (elem == list[i]);
		i++;
	}
	
	return belongs;
}
#endif

static gboolean 
_xvd_keys_handle_events(GIOChannel *source, 
						GIOCondition cond, 
						gpointer data)
{
	XvdInstance *Inst = (XvdInstance *)data;
	
	if (cond | G_IO_IN) {
		xcb_generic_event_t *ev;
		xcb_key_press_event_t *kpe;

		while ((ev = xcb_poll_for_event(Inst->conn))) {
			switch (ev->response_type & ~0x80) {
				case XCB_KEY_PRESS:
					kpe = (xcb_key_press_event_t *)ev;
					
					#ifndef LEGACY_XCBKEYSYMS
					if (_xvd_keys_is_symbol(kpe->detail, Inst->keyRaise)) {
                    #else
					if (kpe->detail == Inst->keyRaise) {
                    #endif
					    #ifndef NDEBUG
                		g_debug ("The RaiseVolume key was pressed.\n");
                		#endif
						if (xvd_mixer_change_volume (Inst, Inst->vol_step)) {
							#ifdef HAVE_LIBNOTIFY
/*							if (!Inst->muted) {*/
							if (Inst->current_vol == 100)
								xvd_notify_overshoot_notification (Inst);
							else
								xvd_notify_volume_notification (Inst);
/*							}*/
							#endif
						}
					}

					#ifndef LEGACY_XCBKEYSYMS
					else if (_xvd_keys_is_symbol(kpe->detail, Inst->keyLower)) {
                    #else
					else if (kpe->detail == Inst->keyLower) {
                    #endif
					    #ifndef NDEBUG
                		g_debug ("The LowerVolume key was pressed.\n");
                		#endif
						if (xvd_mixer_change_volume (Inst, (Inst->vol_step * -1))) {
							#ifdef HAVE_LIBNOTIFY
/*							if (!Inst->muted) {*/
							if (Inst->current_vol == 0)
								xvd_notify_undershoot_notification (Inst);
							else
								xvd_notify_volume_notification (Inst);
/*							}*/
							#endif
						}
					}

					#ifndef LEGACY_XCBKEYSYMS
					else if (_xvd_keys_is_symbol(kpe->detail, Inst->keyMute)) {
                    #else
					else if (kpe->detail == Inst->keyMute) {
                    #endif
					    #ifndef NDEBUG
                		g_debug ("The Mute key was pressed.\n");
                		#endif
						if (xvd_mixer_toggle_mute (Inst)) {
							#ifdef HAVE_LIBNOTIFY
							if (Inst->muted)
								xvd_notify_notification (Inst, "audio-volume-muted", 0);
							else {
								xvd_mixer_init_volume (Inst);
								xvd_notify_volume_notification (Inst);
							}
							#endif
						}
					}
					break;
				
				default :
					break;
			}

			free (ev);
		}
	}

	return TRUE;
}

void 
xvd_keys_init(XvdInstance *Inst)
{
	int 				screennum, 
						i;
	char 				*display = 		NULL;
	const xcb_setup_t 	*setup = 		NULL;
	xcb_screen_t 		*screen = 		NULL;
	xcb_void_cookie_t 	cookie;
	xcb_generic_error_t *error = 		NULL;
	uint16_t 			mod = 			0;
	
	/* Get the display and connect to it */
	display = getenv ("DISPLAY");
	if (!display) {
		g_warning ("No DISPLAY variable set - X11 plugin disabled\n");
		return;
	}

	Inst->conn = xcb_connect (display, &screennum);
	if (!Inst->conn) {
		g_warning ("Could not open display %s - X11 plugin disabled\n", display);
		return;
	}
	
	if (xcb_connection_has_error (Inst->conn)) {
		g_warning ("XCB error while connecting\n");
		return;
	}

	/* Init the window and symbols vars */
	setup = xcb_get_setup (Inst->conn);

	xcb_screen_iterator_t it = xcb_setup_roots_iterator (setup);
	for (i = 0; i < screennum; i++)
		xcb_screen_next (&it);
	screen = it.data;

	Inst->root_win = screen->root;
	
	Inst->kss = xcb_key_symbols_alloc (Inst->conn);

	/* Grab the XF86AudioRaiseVolume key */
	#ifndef LEGACY_XCBKEYSYMS
	Inst->keyRaise = xcb_key_symbols_get_keycode (Inst->kss, XF86XK_AudioRaiseVolume);
	i = 0;
	
	while (Inst->keyRaise[i] != XCB_NO_SYMBOL) {
		cookie = xcb_grab_key_checked (Inst->conn, TRUE, Inst->root_win, 
										mod, Inst->keyRaise[i], 
										XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
		error = xcb_request_check (Inst->conn, cookie);
		if (error) {
			fprintf (stderr, "XCB: Unable to bind RaiseVolume keycode=%d mod=0x%04x: %d\n",
			Inst->keyRaise[i], mod, error->error_code);
		}
		else {
			g_print ("XCB: RaiseVolume ok, keycode=%d mod=0x%04x\n",
			Inst->keyRaise[i], mod);
		}
		i++;
	}

	/* Grab the XF86AudioLowerVolume key */
	Inst->keyLower = xcb_key_symbols_get_keycode (Inst->kss, XF86XK_AudioLowerVolume);
	i = 0;
	
	while (Inst->keyLower[i] != XCB_NO_SYMBOL) {
		cookie = xcb_grab_key_checked (Inst->conn, TRUE, Inst->root_win, 
										mod, Inst->keyLower[i], 
										XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
		error = xcb_request_check (Inst->conn, cookie);
		if (error) {
			fprintf (stderr, "XCB: Unable to bind LowerVolume keycode=%d mod=0x%04x: %d\n",
			Inst->keyLower[i], mod, error->error_code);
		}
		else {
			g_print ("XCB: LowerVolume ok, keycode=%d mod=0x%04x\n",
			Inst->keyLower[i], mod);
		}
		i++;
	}

	
	/* Grab the XF86AudioMute key */
	Inst->keyMute = xcb_key_symbols_get_keycode (Inst->kss, XF86XK_AudioMute);
	i = 0;
	
	while (Inst->keyMute[i] != XCB_NO_SYMBOL) {
		cookie = xcb_grab_key_checked (Inst->conn, TRUE, Inst->root_win, 
										mod, Inst->keyMute[i], 
										XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
		error = xcb_request_check (Inst->conn, cookie);
		if (error) {
			fprintf (stderr, "XCB: Unable to bind Mute keycode=%d mod=0x%04x: %d\n",
			Inst->keyMute[i], mod, error->error_code);
		}
		else {
			g_print ("XCB: Mute ok, keycode=%d mod=0x%04x\n",
			Inst->keyMute[i], mod);
		}
		i++;
	}
	#else
	Inst->keyRaise = xcb_key_symbols_get_keycode (Inst->kss, XF86XK_AudioRaiseVolume);
	
	cookie = xcb_grab_key_checked (Inst->conn, TRUE, Inst->root_win, 
									mod, Inst->keyRaise, 
									XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
	error = xcb_request_check (Inst->conn, cookie);
	if (error) {
		fprintf (stderr, "XCB: Unable to bind RaiseVolume keycode=%d mod=0x%04x: %d\n",
		Inst->keyRaise, mod, error->error_code);
	}
	else {
		g_print ("XCB: RaiseVolume ok, keycode=%d mod=0x%04x\n",
		Inst->keyRaise, mod);
	}

	/* Grab the XF86AudioLowerVolume key */
	Inst->keyLower = xcb_key_symbols_get_keycode (Inst->kss, XF86XK_AudioLowerVolume);
	
	cookie = xcb_grab_key_checked (Inst->conn, TRUE, Inst->root_win, 
									mod, Inst->keyLower, 
									XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
	error = xcb_request_check (Inst->conn, cookie);
	if (error) {
		fprintf (stderr, "XCB: Unable to bind LowerVolume keycode=%d mod=0x%04x: %d\n",
		Inst->keyLower, mod, error->error_code);
	}
	else {
		g_print ("XCB: LowerVolume ok, keycode=%d mod=0x%04x\n",
		Inst->keyLower, mod);
	}

	
	/* Grab the XF86AudioMute key */
	Inst->keyMute = xcb_key_symbols_get_keycode (Inst->kss, XF86XK_AudioMute);
	
	cookie = xcb_grab_key_checked (Inst->conn, TRUE, Inst->root_win, 
									mod, Inst->keyMute, 
									XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
	error = xcb_request_check (Inst->conn, cookie);
	if (error) {
		fprintf (stderr, "XCB: Unable to bind Mute keycode=%d mod=0x%04x: %d\n",
		Inst->keyMute, mod, error->error_code);
	}
	else {
		g_print ("XCB: Mute ok, keycode=%d mod=0x%04x\n",
		Inst->keyMute, mod);
	}
	#endif

	GIOChannel *channel = g_io_channel_unix_new (xcb_get_file_descriptor (Inst->conn));
	g_io_add_watch (channel, G_IO_IN|G_IO_HUP, _xvd_keys_handle_events, Inst);
}

void 
xvd_keys_release (XvdInstance *Inst)
{
	#ifndef LEGACY_XCBKEYSYMS
	g_free (Inst->keyRaise);
	g_free (Inst->keyLower);
	g_free (Inst->keyMute);
	#endif
	if (Inst->kss)
		xcb_key_symbols_free (Inst->kss);
	if (Inst->conn)
		xcb_disconnect (Inst->conn);
}
