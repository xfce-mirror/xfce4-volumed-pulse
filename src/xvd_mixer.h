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

#ifndef _XVD_MIXER_H
#define _XVD_MIXER_H

#include <gst/audio/mixerutils.h>

#include "xvd_data_types.h"

void 
xvd_mixer_init(XvdInstance *Inst);

#ifdef HAVE_LIBNOTIFY
void 
xvd_mixer_init_bus(XvdInstance *Inst);
#endif

void 
xvd_mixer_init_volume(XvdInstance *Inst);

void 
xvd_get_card_from_mixer(XvdInstance *Inst, 
						const gchar *wanted_card,
						const gchar *preferred_fallback);

void 
xvd_get_track_from_mixer(XvdInstance *Inst, 
							const gchar *wanted_track,
							const gchar *preferred_fallback);

void 
xvd_clean_card_name(XvdInstance *Inst);

void 
xvd_clean_cards(XvdInstance *Inst);

#ifdef HAVE_LIBNOTIFY
void
xvd_clean_mixer_bus(XvdInstance *Inst);
#endif

void 
xvd_clean_track(XvdInstance *Inst);

void 
xvd_calculate_avg_volume(XvdInstance *Inst, 
						 gint *volumes, 
						 gint num_channels);

gboolean  
xvd_mixer_change_volume(XvdInstance *Inst, 
						gint step);

gboolean 
xvd_mixer_toggle_mute(XvdInstance *Inst);

#endif
