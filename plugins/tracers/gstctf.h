/* GstShark - A Front End for GstTracer
 * Copyright (C) 2016 RidgeRun Engineering <manuel.leiva@ridgerun.com>
 *
 * This file is part of GstShark.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __GST_CTF_H__
#define __GST_CTF_H__

#include <gst/gst.h>
G_BEGIN_DECLS
    void generate_data_stream_header (FILE * fd, char *UUID, int UUID_size,
    uint32_t stream_id);
void generate_metadata (FILE * fd, int major, int minor, char *UUID,
    int byte_order);
void add_metadata_event_struct (FILE * fd, const char *metadata_event, int id,
    int stream_id);
void do_print_cpuusage_event (FILE * fd, int16_t event_id, uint32_t timestamp,
    uint32_t cpunum, uint64_t cpuload);
void do_print_proctime_event (FILE * fd, int16_t event_id, uint32_t timestamp,
    char *elementname, uint64_t time);
void do_print_framerate_event (FILE * fd, int16_t event_id, uint32_t timestamp,
    char *padname, uint64_t fps);
void do_print_interlatency_event (FILE * fd, int16_t event_id,
    uint32_t timestamp, char *originpad, char *destinationpad, uint64_t time);
void do_print_scheduling_event (FILE * fd, int16_t event_id, uint32_t timestamp,
    char *elementname, uint64_t time);
void do_print_init_timer (FILE * fd, int16_t event_id, uint32_t timestamp,
    uint32_t timer);
G_END_DECLS
#endif /*__GST_CTF_H__*/
