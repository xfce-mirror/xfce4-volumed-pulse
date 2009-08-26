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
#include "xvd_data_types.h"
#include "xvd_mixer.h"
#include "xvd_xfconf.h"
#ifdef HAVE_LIBNOTIFY
#include "xvd_notify.h"
#endif

XvdInstance     *Inst = NULL;

static void 
xvd_daemonize()
{
	gint pid;
	FILE *checkout = NULL;
	
	pid = fork ();
	
	if (-1 == pid) {
		g_warning ("Failed to fork the daemon. Continuing in non-daemon mode.\n");
	} else if (pid > 0) {
		exit (EXIT_SUCCESS);
	}

	pid = setsid ();
	if (pid < 0) {
		exit (EXIT_FAILURE);
	}

	if ((chdir ("/")) < 0) {
		exit (EXIT_FAILURE);
	}

	checkout = freopen ("/dev/null", "r", stdin);
	if (NULL == checkout)
		g_warning("Error when redirecting stdin to /dev/null\n");

	checkout = freopen ("/dev/null", "w", stdout);
	if (NULL == checkout)
		g_warning("Error when redirecting stdout to /dev/null\n");

	checkout = freopen ("/dev/null", "w", stderr);
	if (NULL == checkout)
		g_warning("Error when redirecting stderr to /dev/null\n");
}

static void 
xvd_shutdown()
{
	#ifdef HAVE_LIBNOTIFY
	xvd_clean_mixer_bus (Inst);
	xvd_notify_uninit ();
	#endif
	
	xvd_clean_card_name (Inst);
	xvd_clean_cards (Inst);
	xvd_clean_track (Inst);
	xvd_keys_release (Inst);
	xvd_xfconf_shutdown (Inst);
}

static void 
xvd_instance_init(XvdInstance *i)
{
	i->mixers = NULL;
	i->card = NULL;
	i->card_name = NULL;
	i->track = NULL;
	i->track_label = NULL;
	i->error = NULL;
	i->chan = NULL;
	i->bus = NULL;
	i->bus_id = 0;
	i->loop = NULL;
	i->current_vol = 0;
	i->muted = FALSE;
	i->conn = NULL;
	i->keyRaise = NULL;
	i->keyLower = NULL;
	i->keyMute = NULL;
	i->kss = NULL;
	i->notifyosd = FALSE;
}

gint 
main(gint argc, gchar **argv)
{
	Inst = g_malloc (sizeof (XvdInstance));
	xvd_instance_init (Inst);
	/* Daemon mode */
	#ifdef NDEBUG
	xvd_daemonize();
	#endif

	/* Gstreamer init */
	gst_init (NULL,NULL);

	/* Xcb init */
	xvd_keys_init (Inst);

	/* Xfconf init */
	xvd_xfconf_init (Inst);
	
	/* Get card/track from xfconf */
	Inst->card_name = xvd_get_xfconf_card (Inst);
	
	if (NULL == Inst->card_name) {
		g_warning ("There seems to be no active card defined in the xfce mixer.\n");
	}
	
	/* Mixer init */
	xvd_mixer_init (Inst);
	#ifdef HAVE_LIBNOTIFY
	xvd_mixer_init_bus (Inst);
	#endif
	
	// A mutex for the track might help in very unlikely cases.
	xvd_get_xfconf_card_from_mixer (Inst);
	
	gchar *tmp_track = xvd_get_xfconf_track (Inst);
	xvd_get_xfconf_track_from_mixer (Inst, tmp_track);
	g_free (tmp_track);
	
	xvd_mixer_init_volume (Inst);
	xvd_load_xfconf_vol_step (Inst);
	
	/* Libnotify init and idle till ready for the main loop */
	g_set_application_name (XVD_APPNAME);
	#ifdef HAVE_LIBNOTIFY
	xvd_notify_init (Inst, XVD_APPNAME);
	#endif
	
	while (Inst->xvd_init_error) {
		sleep (1);
	}
	
	
	Inst->loop = g_main_loop_new (NULL, FALSE);
	g_main_loop_run (Inst->loop);
	
	xvd_shutdown ();
	return 0;
}

