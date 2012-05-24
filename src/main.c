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
#include "config.h"
#endif

#include "xvd_keys.h"
#include "xvd_data_types.h"
#include "xvd_pulse.h"
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
	xvd_close_pulse (Inst);
	
	#ifdef HAVE_LIBNOTIFY
	xvd_notify_uninit (Inst);
	#endif
	
	xvd_keys_release (Inst);
	xvd_xfconf_shutdown (Inst);
	
	//TODO xvd_instance_free
}

static void 
xvd_instance_init(XvdInstance *i)
{
	i->pa_main_loop = NULL;
	i->pulse_context = NULL;
	i->sink_index = -1;
	i->error = NULL;
	i->chan = NULL;
	i->loop = NULL;
	#ifdef HAVE_LIBNOTIFY
	i->gauge_notifications = FALSE;
	i->notification	= NULL;
	#endif
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
  
  gtk_init(&argc, &argv);

	/* Grab the keys */
	xvd_keys_init (Inst);

	/* Xfconf init */
	xvd_xfconf_init (Inst);
  
	/* Pulse init */
	if (!xvd_open_pulse (Inst))
	{
		g_warning ("Unable to initialize pulseaudio support, quitting");
		xvd_shutdown ();
		return 1;
	}
	
	xvd_xfconf_get_vol_step (Inst);
	
	/* Libnotify init and idle till ready for the main loop */
	g_set_application_name (XVD_APPNAME);
	#ifdef HAVE_LIBNOTIFY
	xvd_notify_init (Inst, XVD_APPNAME);
	#endif
	
	Inst->loop = g_main_loop_new (NULL, FALSE);
	g_main_loop_run (Inst->loop);
	
	xvd_shutdown ();
	return 0;
}

