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

#ifndef _XVD_PULSE_H
#define _XVD_PULSE_H

#include <pulse/volume.h>

#include "xvd_data_types.h"


/**
 * Entry point, required to use the pulseaudio server.
 */
gboolean xvd_open_pulse          (XvdInstance        *i);

/**
 * Exit(?!) point, to clean up.
 */
void     xvd_close_pulse         (XvdInstance        *i);

/**
 * Changes the volume in the given direction.
 */
void     xvd_update_volume       (XvdInstance        *i,
                                  XvdVolStepDirection d);

/**
 * Toggle mute.
 */
void     xvd_toggle_mute         (XvdInstance        *i);

/**
 * Returns a percentage volume (i.e. between 0 and 100, usable on notifications)
 */
gint     xvd_get_readable_volume (const pa_cvolume   *vol);

#endif
