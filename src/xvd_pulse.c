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
#include <pulse/volume.h>

#include "xvd_pulse.h"


#ifdef HAVE_LIBNOTIFY
static void xvd_notify_volume_update       (pa_context                     *c,
                                            int                             success,
                                            void                           *userdata);
#else
#define xvd_notify_volume_update NULL
#endif

#ifdef HAVE_LIBNOTIFY
static void xvd_notify_volume_mute         (pa_context                     *c,
                                            int                             success,
                                            void                           *userdata);
#else
#define xvd_notify_volume_mute NULL
#endif

#ifdef HAVE_LIBNOTIFY
static guint    xvd_get_readable_volume    (const pa_cvolume               *vol);
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


void
xvd_update_volume (XvdInstance        *i,
                   XvdVolStepDirection d)
{
  pa_operation *op = NULL;
  pa_cvolume  *new_volume = NULL;

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
    }

  switch (d)
    {
      case XVD_UP:
        new_volume = pa_cvolume_inc_clamp (&i->volume,
                                           PA_VOL_STEP_DEFAULT,
                                           PA_VOLUME_NORM);
      break;
      case XVD_DOWN:
        new_volume = pa_cvolume_dec (&i->volume,
                                     PA_VOL_STEP_DEFAULT);
      break;
      default:
        g_warning ("xvd_update_volume: invalid direction");
        return;
      break;
    }

#ifdef HAVE_LIBNOTIFY
  i->new_vol = xvd_get_readable_volume ((const pa_cvolume *)new_volume);
#endif

  op = pa_context_set_sink_volume_by_index (i->pulse_context,
                                            i->sink_index,
                                            new_volume,
                                            xvd_notify_volume_update,
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
    }

  op =  pa_context_set_sink_mute_by_index (i->pulse_context,
                                           i->sink_index,
                                           !i->mute,
                                           xvd_notify_volume_mute,
                                           i);

  if (!op)
    {
      g_warning ("xvd_toggle_mute: failed");
      return;
    }
  pa_operation_unref (op);
  i->mute = !i->mute;
}


#ifdef HAVE_LIBNOTIFY
static void
xvd_notify_volume_update (pa_context *c,
                          int         success,
                          void       *userdata)
{
  XvdInstance  *i = (XvdInstance *) userdata;

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

  if (i->current_vol >= 100 && i->new_vol >= i->current_vol)
    {
      i->current_vol = i->new_vol;
      xvd_notify_overshoot_notification (i);
    }
  else if (i->current_vol <= 0 && i->new_vol <= i->current_vol)
   {
     i->current_vol = i->new_vol;
     xvd_notify_undershoot_notification (i);
   }
  else
   {
     i->current_vol = i->new_vol;
     xvd_notify_volume_notification (i);
   }
}
#endif


#ifdef HAVE_LIBNOTIFY
static void
xvd_notify_volume_mute (pa_context *c,
                        int         success,
                        void       *userdata)
{
  XvdInstance  *i = (XvdInstance *) userdata;

  if (!c || !userdata)
    {
      g_warning ("xvd_notify_volume_mute: invalid argument");
      return;
    }

  if (!success)
    {
      g_warning ("xvd_notify_volume_mute: operation failed, %s",
                 pa_strerror (pa_context_errno (c)));
      return;
    }

  if (i->mute)
    xvd_notify_notification (i, "audio-volume-muted", i->current_vol);
  else
    xvd_notify_volume_notification (i);
}
#endif


static void 
xvd_subscribed_events_callback (pa_context                     *c,
                                enum pa_subscription_event_type t,
                                uint32_t                        index,
                                void                           *userdata)
{
  XvdInstance  *i = (XvdInstance *) userdata;
  pa_operation *op = NULL;

  if (!userdata)
    {
      g_critical ("xvd_subscribed_events_callback: invalid argument");
      return;
    }

  switch (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK)
    {
      case PA_SUBSCRIPTION_EVENT_SINK:
        if (i->sink_index != index)
          return;

        if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE)
          i->sink_index == PA_INVALID_INDEX;
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


static void
xvd_context_state_callback (pa_context *c,
                            void       *userdata)
{
  XvdInstance           *i = (XvdInstance *) userdata;
  pa_subscription_mask_t mask = PA_SUBSCRIPTION_MASK_SINK | PA_SUBSCRIPTION_MASK_SERVER;
  pa_operation          *op = NULL;

  if (!userdata)
    {
      g_critical ("xvd_context_state_callback: invalid argument");
      return;
    }

  switch (pa_context_get_state (c))
    {
      case PA_CONTEXT_FAILED:
        g_critical("xvd_context_state_callback: PA_CONTEXT_FAILED, is PulseAudio Daemon running?");
        return;
      break;
      case PA_CONTEXT_READY:
        pa_context_set_subscribe_callback (c,
                                           xvd_subscribed_events_callback,
                                           userdata);

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


static void 
xvd_server_info_callback (pa_context           *c,
                          const pa_server_info *info,
                          void                 *userdata)
{
  pa_operation *op = NULL;

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

      if (i->sink_index == PA_INVALID_INDEX
          /* indicator-sound does that check */
          && g_ascii_strncasecmp ("auto_null", sink->name, 9) != 0)
        {
          i->sink_index = sink->index;
          i->volume = sink->volume;
          i->mute = sink->mute;
#ifdef HAVE_LIBNOTIFY
  i->current_vol = xvd_get_readable_volume ((const pa_cvolume *)&sink->volume);
#endif
        }
    }
}


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

      if (i->sink_index != info->index)
        {
          i->sink_index = info->index;
          i->volume = info->volume;
          i->mute = info->mute;
#ifdef HAVE_LIBNOTIFY
  i->current_vol = xvd_get_readable_volume ((const pa_cvolume *)&info->volume);
#endif
        }
    }
}

static void
xvd_update_sink_callback (pa_context         *c,
                          const pa_sink_info *info,
                          int                 eol,
                          void               *userdata)
{
  XvdInstance *i = (XvdInstance *) userdata;
#ifdef HAVE_LIBNOTIFY
  int          mute = 0;
#endif

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
#ifdef HAVE_LIBNOTIFY
      i->new_vol = xvd_get_readable_volume ((const pa_cvolume *)&info->volume);
#endif
      i->sink_index = info->index;
      i->volume = info->volume;
#ifdef HAVE_LIBNOTIFY
      if (i->new_vol != i->current_vol)
        xvd_notify_volume_update (NULL, 1, i);
      mute = i->mute;
#endif
      i->mute = info->mute;
#ifdef HAVE_LIBNOTIFY
     if (mute != info->mute)
       xvd_notify_volume_mute (NULL, 1, i);
#endif
    }
}

#ifdef HAVE_LIBNOTIFY
/**
 * Returns a volume usable on notifications.
 */
static guint
xvd_get_readable_volume (const pa_cvolume *vol)
{
  guint new_vol = 0;

  new_vol = 100 * pa_cvolume_avg (vol) / PA_VOLUME_NORM;
  return CLAMP (new_vol, 0, 100);
}
#endif
