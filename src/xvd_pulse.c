/*
 *  xfce4-volumed-pulse - Volume management daemon for XFCE 4 (Pulseaudio variant)
 *
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

#include <pulse/error.h>
#include <pulse/introspect.h>
#include <pulse/subscribe.h>

#include "xvd_pulse.h"
#include "xvd_notify.h"


/**
 * Translates a "human" volume step (0-100) for PulseAudio.
 */
#define XVD_PA_VOLUME_STEP(n) ((pa_volume_t)((n) * PA_VOLUME_NORM / 100))


static pa_cvolume old_volume;
static int        old_mute;


#ifdef HAVE_LIBNOTIFY
static void xvd_notify_volume_callback     (pa_context                     *c,
                                            int                             success,
                                            void                           *userdata);
#else
#define xvd_notify_volume_callback NULL
#endif

static void xvd_context_state_callback     (pa_context                     *c,
                                            void                           *userdata);

static void xvd_subscribed_events_callback (pa_context                     *c,
                                            enum pa_subscription_event_type t,
                                            uint32_t                        index,
                                            void                           *userdata);

static void xvd_server_info_callback       (pa_context                     *c,
                                            const pa_server_info           *info,
                                            void                           *userdata);

static void xvd_default_sink_info_callback (pa_context                     *c,
                                            const pa_sink_info             *info,
                                            int                             eol,
                                            void                           *userdata);

static void xvd_sink_info_callback         (pa_context                     *c,
                                            const pa_sink_info             *sink,
                                            int                             eol,
                                            void                           *userdata);

static void xvd_update_sink_callback       (pa_context                     *c,
                                            const pa_sink_info             *info,
                                            int                             eol,
                                            void                           *userdata);

static gboolean xvd_connect_to_pulse       (XvdInstance                    *i);


gboolean
xvd_open_pulse (XvdInstance *i)
{
  i->pa_main_loop = pa_glib_mainloop_new (NULL);
  g_assert (i->pa_main_loop);
  return xvd_connect_to_pulse (i);
}


void
xvd_close_pulse (XvdInstance *i)
{
  if (i->pulse_context)
    {
      pa_context_unref (i->pulse_context);
      i->pulse_context = NULL;
    }
  pa_glib_mainloop_free (i->pa_main_loop);
  i->pa_main_loop = NULL;
}


void
xvd_update_volume (XvdInstance        *i,
                   XvdVolStepDirection d)
{
  pa_operation *op = NULL;

  if (!i || !i->pulse_context)
    {
      g_warning ("xvd_update_volume: pulseaudio context is null");
      return;
    }

  if (pa_context_get_state (i->pulse_context) != PA_CONTEXT_READY)
    {
      g_warning ("xvd_update_volume: pulseaudio context isn't ready");
      return;
    }

  if (i->sink_index == PA_INVALID_INDEX)
    {
      g_warning ("xvd_update_volume: undefined sink");
      return;
    }

  /* backup */
  old_volume = i->volume;

  switch (d)
    {
      case XVD_UP:
        pa_cvolume_inc_clamp (&i->volume,
                              XVD_PA_VOLUME_STEP(i->vol_step),
                              PA_VOLUME_NORM);
      break;
      case XVD_DOWN:
        pa_cvolume_dec (&i->volume,
                        XVD_PA_VOLUME_STEP(i->vol_step));
      break;
      default:
        g_warning ("xvd_update_volume: invalid direction");
        return;
      break;
    }

  op = pa_context_set_sink_volume_by_index (i->pulse_context,
                                            i->sink_index,
                                            &i->volume,
                                            xvd_notify_volume_callback,
                                            i);

  if (!op)
    {
      g_warning ("xvd_update_volume: failed");
      return;
    }
  pa_operation_unref (op);
}


void
xvd_toggle_mute (XvdInstance *i)
{
  pa_operation *op = NULL;

  if (!i || !i->pulse_context)
   {
      g_warning ("xvd_toggle_mute: pulseaudio context is null");
      return;
   }

  if (pa_context_get_state (i->pulse_context) != PA_CONTEXT_READY)
    {
      g_warning ("xvd_toggle_mute: pulseaudio context isn't ready");
      return;
    }

  if (i->sink_index == PA_INVALID_INDEX)
    {
      g_warning ("xvd_toggle_mute: undefined sink");
      return;
    }

  /* backup existing mute and update */
  i->mute = !(old_mute = i->mute);

  op =  pa_context_set_sink_mute_by_index (i->pulse_context,
                                           i->sink_index,
                                           i->mute,
                                           xvd_notify_volume_callback,
                                           i);

  if (!op)
    {
      g_warning ("xvd_toggle_mute: failed");
      return;
    }
  pa_operation_unref (op);
}


gint
xvd_get_readable_volume (const pa_cvolume *vol)
{
  guint new_vol = 0;

  new_vol = 100 * pa_cvolume_avg (vol) / PA_VOLUME_NORM;
  return MIN (new_vol, 100);
}


/**
 * This function does the context initialization.
 */
static gboolean
xvd_connect_to_pulse (XvdInstance *i)
{
  pa_context_flags_t flags = PA_CONTEXT_NOFAIL;

  if (i->pulse_context)
    {
      pa_context_unref (i->pulse_context);
      i->pulse_context = NULL;
    }

  i->pulse_context = pa_context_new (pa_glib_mainloop_get_api (i->pa_main_loop),
                                     XVD_APPNAME);
  g_assert(i->pulse_context);
  pa_context_set_state_callback (i->pulse_context,
                                 xvd_context_state_callback,
                                 i);

  if (pa_context_connect (i->pulse_context,
                          NULL,
                          flags,
                          NULL) < 0)
    {
      g_warning ("xvd_connect_to_pulse: failed to connect context: %s",
                 pa_strerror (pa_context_errno (i->pulse_context)));
      return FALSE;
    }
  return TRUE;
}


#ifdef HAVE_LIBNOTIFY
/**
 * Decides the type of notification to show on a change.
 */
static void
xvd_notify_volume_callback (pa_context *c,
                            int         success,
                            void       *userdata)
{
  XvdInstance  *i = (XvdInstance *) userdata;
  guint32       r_oldv, r_curv;

  if (!c || !userdata)
    {
      g_warning ("xvd_notify_volume_update: invalid argument");
      return;
    }

  if (!success)
    {
      g_warning ("xvd_notify_volume_update: operation failed, %s",
                 pa_strerror (pa_context_errno (c)));
      return;
    }

  /* the sink was (un)muted */
  if (old_mute != i->mute)
    {
      xvd_notify_volume_notification (i);
      return;
    }

  r_oldv = xvd_get_readable_volume (&old_volume);
  r_curv = xvd_get_readable_volume (&i->volume);

  /* trying to go above 100 */
  if (r_oldv == 100 && r_curv >= r_oldv)
    xvd_notify_overshoot_notification (i);
  /* trying to go below 0 */
  else if (r_oldv == 0 && r_curv <= r_oldv)
   xvd_notify_undershoot_notification (i);
  /* normal */
  else
   xvd_notify_volume_notification (i);
}
#endif


/**
 * Callback to analyze events emitted by the server.
 */
static void
xvd_subscribed_events_callback (pa_context                     *c,
                                enum pa_subscription_event_type t,
                                uint32_t                        index,
                                void                           *userdata)
{
  XvdInstance  *i = (XvdInstance *) userdata;
  pa_operation *op = NULL;

  if (!c || !userdata)
    {
      g_critical ("xvd_subscribed_events_callback: invalid argument");
      return;
    }

  switch (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK)
    {
      /* change on a sink, re-fetch it */
      case PA_SUBSCRIPTION_EVENT_SINK:
        if (i->sink_index != index)
          return;

        if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE)
          i->sink_index = PA_INVALID_INDEX;
        else
          {
             op = pa_context_get_sink_info_by_index (c,
                                                     index,
                                                     xvd_update_sink_callback,
                                                     userdata);

             if (!op)
               {
                 g_warning ("xvd_subscribed_events_callback: failed to get sink info");
                 return;
               }
             pa_operation_unref (op);
          }
      break;
      /* change on the server, re-fetch everything */
      case PA_SUBSCRIPTION_EVENT_SERVER:
        op = pa_context_get_server_info (c,
                                         xvd_server_info_callback,
                                         userdata);

        if (!op)
          {
            g_warning("xvd_subscribed_events_callback: failed to get server info");
            return;
          }
        pa_operation_unref(op);
      break;
    }
}


/**
 * Callback to check the status of context initialization.
 */
static void
xvd_context_state_callback (pa_context *c,
                            void       *userdata)
{
  XvdInstance           *i = (XvdInstance *) userdata;
  pa_subscription_mask_t mask = PA_SUBSCRIPTION_MASK_SINK | PA_SUBSCRIPTION_MASK_SERVER;
  pa_operation          *op = NULL;

  if (!c || !userdata)
    {
      g_critical ("xvd_context_state_callback: invalid argument");
      return;
    }

  switch (pa_context_get_state (c))
    {
      case PA_CONTEXT_UNCONNECTED:
        g_debug ("xvd_context_state_callback: The context hasn't been connected yet");
      break;
      case PA_CONTEXT_CONNECTING:
        g_debug ("xvd_context_state_callback: A connection is being established");
      break;
      case PA_CONTEXT_AUTHORIZING:
        g_debug ("xvd_context_state_callback: The client is authorizing itself to the daemon");
      break;
      case PA_CONTEXT_SETTING_NAME:
        g_debug ("xvd_context_state_callback: The client is passing its application name to the daemon");
      break;
      case PA_CONTEXT_TERMINATED:
        g_debug ("xvd_context_state_callback: The connection was terminated cleanly");
        i->sink_index = PA_INVALID_INDEX;
      break;
      case PA_CONTEXT_FAILED:
        g_critical("xvd_context_state_callback: The connection failed or was disconnected, is PulseAudio Daemon running?");
        i->sink_index = PA_INVALID_INDEX;
      break;
      case PA_CONTEXT_READY:
        g_debug ("xvd_context_state_callback: The connection is established, the context is ready to execute operations");
        pa_context_set_subscribe_callback (c,
                                           xvd_subscribed_events_callback,
                                           userdata);

        /* subscribe to sink and server changes, we don't need more */
        op = pa_context_subscribe (c,
                                   mask,
                                   NULL,
                                   NULL);

        if (!op)
          {
            g_critical ("xvd_context_state_callback: pa_context_subscribe() failed");
            return;
          }
        pa_operation_unref(op);

        op = pa_context_get_server_info (c,
                                         xvd_server_info_callback,
                                         userdata);

        if (!op)
          {
            g_warning("xvd_context_state_callback: pa_context_get_server_info() failed");
            return;
          }
        pa_operation_unref(op);
      break;
    }
}


/**
 * Callback to retrieve server infos (mostly, the default sink).
 */
static void
xvd_server_info_callback (pa_context           *c,
                          const pa_server_info *info,
                          void                 *userdata)
{
  pa_operation *op = NULL;

  if (!c || !userdata)
    {
      g_warning ("xvd_server_info_callback: invalid argument");
      return;
    }

  if (!info)
    {
      g_warning("xvd_server_info_callback: No PulseAudio server");
      return;
    }

  if (info->default_sink_name)
    {
      op = pa_context_get_sink_info_by_name (c,
                                             info->default_sink_name,
                                             xvd_default_sink_info_callback,
                                             userdata);

      if (!op)
        {
          g_warning("xvd_server_info_callback: pa_context_get_sink_info_by_name() failed");
          return;
        }
      pa_operation_unref (op);
    }
  else
    {
      /* when PulseAudio doesn't set a default sink, look at all of them
         and hope to find a usable one */
      op = pa_context_get_sink_info_list(c,
                                         xvd_sink_info_callback,
                                         userdata);

      if (!op)
        {
          g_warning("xvd_server_info_callback: pa_context_get_sink_info_list() failed");
          return;
        }
      pa_operation_unref (op);
    }
}


/**
 * Callback to retrieve the infos of a given sink.
 */
static void
xvd_sink_info_callback (pa_context         *c,
                        const pa_sink_info *sink,
                        int                 eol,
                        void               *userdata)
{
  XvdInstance *i = (XvdInstance *) userdata;

  /* detect the end of the list */
  if (eol > 0)
    return;
  else
    {
      if (!userdata || !sink)
        {
          g_warning ("xvd_sink_info_callback: invalid argument");
          return;
        }

      /* If there's no default sink, try to use this one */
      if (i->sink_index == PA_INVALID_INDEX
          /* indicator-sound does that check */
          && g_ascii_strncasecmp ("auto_null", sink->name, 9) != 0)
        {
          i->sink_index = sink->index;
          old_volume = i->volume = sink->volume;
          old_mute = i->mute = sink->mute;
        }
    }
}


/**
 * Callback to retrieve the infos of the default sink.
 */
static void
xvd_default_sink_info_callback (pa_context         *c,
                                const pa_sink_info *info,
                                int                 eol,
                                void               *userdata)
{
  XvdInstance *i = (XvdInstance *) userdata;

  /* detect the end of the list */
  if (eol > 0)
    return;
  else
    {
      if (!userdata || !info)
        {
          g_warning ("xvd_default_sink_info_callback: invalid argument");
          return;
        }

      /* is this a new default sink? */
      if (i->sink_index != info->index)
        {
          i->sink_index = info->index;
          old_volume = i->volume = info->volume;
          old_mute = i->mute = info->mute;
        }
    }
}


/**
 * Callback for sink changes reported by PulseAudio.
 */
static void
xvd_update_sink_callback (pa_context         *c,
                          const pa_sink_info *info,
                          int                 eol,
                          void               *userdata)
{
  XvdInstance *i = (XvdInstance *) userdata;

  /* detect the end of the list */
  if (eol > 0)
    return;
  else
    {
      if (!c || !userdata || !info)
        {
          g_warning ("xvd_update_sink_callback: invalid argument");
          return;
        }

      /* re-fetch infos from PulseAudio */
      i->sink_index = info->index;
      old_volume = i->volume;
      i->volume = info->volume;
      old_mute = i->mute;
      i->mute = info->mute;

#ifdef HAVE_LIBNOTIFY
      /* notify user of the possible changes */
      xvd_notify_volume_callback (c, 1, i);
#endif
    }
}
