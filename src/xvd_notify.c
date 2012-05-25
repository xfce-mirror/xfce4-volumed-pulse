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

#ifdef HAVE_LIBNOTIFY
#include <libnotify/notify.h>

#include "xvd_notify.h"

#include "xvd_pulse.h"


void 
xvd_notify_notification(XvdInstance *Inst, 
						gchar* icon, 
						gint value)
{
	GError* error						= NULL;
	gchar*  title						= NULL;

	if ((icon != NULL) && (g_strcmp0(icon, "audio-volume-muted") == 0)) {
		// TRANSLATORS: this is the body of the ATK interface of the volume notifications. This is the case when volume is muted
		title = g_strdup ("Volume is muted");
	}
	else {
		// TRANSLATORS: %d is the volume displayed as a percent, and %c is replaced by '%'. If it doesn't fit in your locale feel free to file a bug.
		title = g_strdup_printf ("Volume is at %d%c", value, '%');
	}
	
	notify_notification_update (Inst->notification,
				title,
				NULL,
				icon);
	
	g_free (title);
	
	if (Inst->gauge_notifications) {
		notify_notification_set_hint_int32 (Inst->notification,
							"value",
							value);
		notify_notification_set_hint_string (Inst->notification,
							 "x-canonical-private-synchronous",
							 "");
	}
	
	if (!notify_notification_show (Inst->notification, &error))
	{
		g_warning ("Error while sending notification : %s\n", error->message);
		g_error_free (error);
	}
}

void 
xvd_notify_volume_notification(XvdInstance *Inst)
{
	gint vol = xvd_get_readable_volume (&Inst->volume);
	if (vol == 0)
		xvd_notify_notification (Inst, (Inst->mute) ? "audio-volume-muted" : "audio-volume-off", vol);
	else if (vol < 34)
		xvd_notify_notification (Inst, (Inst->mute) ? "audio-volume-muted" : "audio-volume-low", vol);
	else if (vol < 67)
		xvd_notify_notification (Inst, (Inst->mute) ? "audio-volume-muted" : "audio-volume-medium", vol);
	else
		xvd_notify_notification (Inst, (Inst->mute) ? "audio-volume-muted" : "audio-volume-high", vol);
}

void
xvd_notify_overshoot_notification(XvdInstance *Inst)
{
	xvd_notify_notification (Inst, 
	    (Inst->mute) ? "audio-volume-muted" : "audio-volume-high",
	    (Inst->gauge_notifications) ? 101 : 100);
}

void
xvd_notify_undershoot_notification(XvdInstance *Inst)
{
	xvd_notify_notification (Inst, 
	    (Inst->mute) ? "audio-volume-muted" : "audio-volume-low",
	    (Inst->gauge_notifications) ? -1 : 0);
}

void 
xvd_notify_init(XvdInstance *Inst, 
				const gchar *appname)
{
	Inst->gauge_notifications = TRUE;
	notify_init (appname);
	
	GList *caps_list = notify_get_server_caps ();
	
	if (caps_list)
	{
		GList *node;

		node = g_list_find_custom (caps_list, LAYOUT_ICON_ONLY, (GCompareFunc) g_strcmp0);
		if (!node)
			Inst->gauge_notifications = FALSE;
		
/*		node = g_list_find_custom (caps_list, SYNCHRONOUS, (GCompareFunc) g_strcmp0);*/
/*		if (!node)*/
/*			Inst->gauge_notifications = FALSE;*/
		
		g_list_free (caps_list);
	}
	
#ifdef NOTIFY_CHECK_VERSION
#if NOTIFY_CHECK_VERSION (0, 7, 0)
	Inst->notification = notify_notification_new ("Xfce4-Volumed", NULL, NULL);
#else
	Inst->notification = notify_notification_new ("Xfce4-Volumed", NULL, NULL, NULL);
#endif
#else
	Inst->notification = notify_notification_new ("Xfce4-Volumed", NULL, NULL, NULL);
#endif
}

void 
xvd_notify_uninit (XvdInstance *Inst)
{
	g_object_unref (G_OBJECT (Inst->notification));
	Inst->notification = NULL;
	notify_uninit ();
}
#endif
