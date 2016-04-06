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
   typedef struct _GstCtfDescriptor GstCtfDescriptor;

typedef enum {
    TIMER_INIT_EVENT_ID,
    CPUUSAGE_EVENT_ID,
    PROCTIME_EVENT_ID,
    INTERLATENCY_EVENT_ID,
    FPS_EVENT_ID,
    SCHED_TIME_EVENT_ID,
} event_id;

void gst_vtf_init ();
void gst_ctf_close (); 
void add_metadata_event_struct (const gchar *metadata_event, gint id,
    gint stream_id);
void do_print_cpuusage_event (gint16 event_id, guint32 timestamp,
    guint32 cpunum, guint64 cpuload);
void do_print_proctime_event (gint16 event_id, guint32 timestamp,
    gchar *elementname, uint64_t time);
void do_print_framerate_event (gint16 event_id, guint32 timestamp,
    gchar *padname, guint64 fps);
void do_print_interlatency_event (gint16 event_id,
    guint32 timestamp, char *originpad, gchar *destinationpad, guint64 time);
void do_print_scheduling_event (gint16 event_id, guint32 timestamp,
    gchar *elementname, guint64 time);
void do_print_init_timer (gint16 event_id, guint32 timestamp,
    guint32 timer);
G_END_DECLS
#endif /*__GST_CTF_H__*/
