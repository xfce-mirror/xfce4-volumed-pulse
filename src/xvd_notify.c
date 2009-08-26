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
#include "xvd_notify.h"

void 
xvd_notify_notification(XvdInstance *Inst, 
						gchar* icon, 
						gint value)
{
	if (Inst->notifyosd) {
		NotifyNotification* notification;

		notification = notify_notification_new (
					"Volume",
					NULL,
					icon,
					NULL);
		notify_notification_set_hint_int32 (notification,
							"value",
							value);
		notify_notification_set_hint_string (notification,
							 "x-canonical-private-synchronous",
							 "");
		Inst->error = NULL;
		if (!notify_notification_show (notification, &Inst->error))
		{
			g_warning ("Error while sending notification : %s\n", Inst->error->message);
			g_error_free (Inst->error);
		}
		g_object_unref (G_OBJECT (notification));
	}
}

void 
xvd_notify_volume_notification(XvdInstance *Inst)
{
	if (Inst->current_vol == 0)
		xvd_notify_notification (Inst, "notification-audio-volume-off", 0);
	else if (Inst->current_vol < 34)
		xvd_notify_notification (Inst, "notification-audio-volume-low", Inst->current_vol);
	else if (Inst->current_vol < 67)
		xvd_notify_notification (Inst, "notification-audio-volume-medium", Inst->current_vol);
	else
		xvd_notify_notification (Inst, "notification-audio-volume-high", Inst->current_vol);
}

void
xvd_notify_overshoot_notification(XvdInstance *Inst)
{
	xvd_notify_notification (Inst, "notification-audio-volume-high", 101);
}

void
xvd_notify_undershoot_notification(XvdInstance *Inst)
{
	xvd_notify_notification (Inst, "notification-audio-volume-off", -1);
}

void 
xvd_notify_init(XvdInstance *Inst, 
				const gchar *appname)
{
	Inst->notifyosd = TRUE;
	notify_init (appname);
	
	GList *caps_list = notify_get_server_caps ();
	
	if (caps_list)
	{
		GList *node;

		node = g_list_find_custom (caps_list, LAYOUT_ICON_ONLY, (GCompareFunc) g_strcmp0);
		if (!node)
			Inst->notifyosd = FALSE;
		
		node = g_list_find_custom (caps_list, SYNCHRONOUS, (GCompareFunc) g_strcmp0);
		if (!node)
			Inst->notifyosd = FALSE;
		
		g_list_free (caps_list);
	}
}

void 
xvd_notify_uninit ()
{
	notify_uninit ();
}
#endif
