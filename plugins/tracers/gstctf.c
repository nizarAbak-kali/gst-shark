/* GstShark - A Front End for GstTracer
 * Copyright (C) 2016 RidgeRun Engineering <manuel.leiva@ridgerun.com>
 *                                         <sebastian.fatjo@ridgerun.com>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <glib/gstdint.h>
#include <glib/gprintf.h>

#include "gstctf.h"

typedef enum {
  BYTE_ORDER_BE,
  BYTE_ORDER_LE,
} byte_order;

struct _GstCtfDescriptor
{
  FILE * metadata;
  FILE * datastream;
  GMutex *mutex;
  GstClockTime start_time;
};

static GstCtfDescriptor *ctf_descriptor = NULL;

/* Metadata format string */
static const char metadata_fmt[] = "/* CTF 1.8 */\n\
typealias integer { size = 8; align = 8; signed = false; } := uint8_t;\n\
typealias integer { size = 16; align = 8; signed = false; } := uint16_t;\n\
typealias integer { size = 32; align = 8; signed = false; } := uint32_t;\n\
typealias integer { size = 64; align = 8; signed = false; } := uint64_t;\n\
\n\
trace {\n\
	major = %u;\n\			/* major (e.g. 0) */
	minor = %u;\n\			/* minor (e.g. 1) */
	uuid = \"%s\";\n\		/* UUID */
	byte_order = %s;\n\		/* be or le */
	packet.header := struct {\n\
		uint32_t magic;\n\
		uint8_t  uuid[16];\n\
		uint32_t stream_id;\n\
	};\n\
};\n\
\n\
typealias integer {\n\
	size = 32; align = 8; signed = false;\n\
	map = clock.monotonic.value;\n\
} := uint32_clock_monotonic_t;\n\
\n\
typealias integer {\n\
	size = 64; align = 8; signed = false;\n\
	map = clock.monotonic.value;\n\
} := uint64_clock_monotonic_t;\n\
\n\
struct packet_context {\n\
	uint64_clock_monotonic_t timestamp_begin;\n\
	uint64_clock_monotonic_t timestamp_end;\n\
	uint32_t events_discarded;\n\
	uint32_t cpu_id;\n\
};\n\
\n\
struct event_header {\n\
	enum : uint16_t { compact = 0 ... 65534, extended = 65535 } id;\n\
	variant <id> {\n\
		struct {\n\
			uint64_clock_monotonic_t timestamp;\n\
		} compact;\n\
		struct {\n\
			uint32_t id;\n\
			uint64_clock_monotonic_t timestamp;\n\
		} extended;\n\
	} v;\n\
} align(8);\n\
\n\
stream {\n\
	id = 0;\n\
	event.header := struct event_header;\n\
	packet.context := struct packet_context;\n\
};\n\
\n";

gboolean gst_ctf_init () {
  gchar UUID[] = {0xd1,0x8e,0x63,0x74,0x35,0xa1,0xcd,0x42,0x8e,0x70,0xa9,0xcf,0xfa,0x71,0x27,0x93};
  gchar * UUIDstring;

  g_sprintf (UUIDstring, "d18e6374-35a1-cd42-8e70-a9cffa712793");

  if (tracer_ctf) {
    GST_ERROR ("@SFC: Error! Structure already exits!");
    return FALSE;
  }

  ctf_descriptor = create_new_ctf();
  generate_datastream_header(UUID, sizeof (UUID), 0);
  generate_metadata(1, 3, UUIDstring, BYTE_ORDER_LE);

  g_free (UUIDstring);

  return TRUE;
}

void gst_ctf_close () {
  fclose (ctf_descriptor->metadata);
  fclose (ctf_descriptor->datastream);
  g_mutex_clear (ctf_descriptor->mutex);
  g_free (ctf_descriptor);
}

static GstCtfDescriptor create_new_ctf () {
  GstCtfDescriptor ctf;
  gchar *dir_name;
  gchar *metadata_file;
  gchar *datastream_file;
  time_t now = time (NULL);

  g_sprintf (metadata_file, "metadata");
  g_sprintf (datastream_file, "datastream");

  /* Creating the output folder for the CTF output files. */

  g_date_strftime (dir_name, 30, "gstshark_ctf_%Y%m%d%H%M%S", localtime(&now));

  if (!g_file_test (dir_name, G_FILE_TEST_EXISTS)) {
    GST_ERROR ("@SFC: Creating %s directory.", dir_name);
    g_mkdir(dir_name, 0666);
  } else {
    GST_ERROR ("@SFC: Directory %s already exists.", dir_name);
  }

  /* Allocating memory space for the private structure that will 
     contains the file descriptors for the CTF ouput. */
  ctf = malloc (sizeof (GstCtfDescriptor));

  ctf->datastream = g_fopen (datastream_file, "a");
  ctf->metadata = g_fopen (metadata_file, "a");
  g_mutex_init (ctf->mutex);

  g_free (dir_name);
  g_free (datastream_file);
  g_free (metadata_file);

  return ctf;
}

static void
generate_datastream_header (gchar *UUID, gint UUID_size,
    guint32 stream_id)
{
  guint64 time_stamp_begin;
  guint64 time_stamp_end;
  guint32 events_discarted;
  guint32 cpu_id;
  gint Magic = 0xC1FC1FC1;

  /* The begin of the data stream header is compound by the Magic Number,
     the trace UUID and the Stream ID. These are all required fields. */

  g_mutex_lock(ctf_descriptor->mutex);
  /* Magic Number */
  fwrite (&Magic, sizeof (gchar), sizeof (gint), ctf_descriptor->datastream);

  /* Trace UUID */
  fwrite (UUID, sizeof (gchar), UUID_size, ctf_descriptor->datastream);

  /* Stream ID */
  fwrite (&stream_id, sizeof (gchar), sizeof (guint32), ctf_descriptor->datastream);

  /* The following bytes correspond to the event packet context, these 
     fields are optional. */

  /* Time Stamp begin */
  time_stamp_begin = 0x3e3db41faf8;     // 0xf8fa41dbe3030000
  fwrite (&time_stamp_begin, sizeof (gchar), sizeof (guint64), ctf_descriptor->datastream);

  /* Time Stamp end */
  time_stamp_end = 0x000003e3ec8152ee;  // 0xee5281ece3030000;
  fwrite (&time_stamp_end, sizeof (gchar), sizeof (guint64), ctf_descriptor->datastream);

  /* Events discarted */
  events_discarted = 0x0;
  fwrite (&events_discarted, sizeof (gchar), sizeof (guint32), ctf_descriptor->datastream);

  /* CPU ID */
  cpu_id = 0x0;
  fwrite (&cpu_id, sizeof (gchar), sizeof (guint32), ctf_descriptor->datastream);

  /* Padding needed */
  guint32 unknown = 0x0000FFFF;
  fwrite (&unknown, sizeof (gchar), sizeof (guint32), ctf_descriptor->datastream);

  g_mutex_unlock(ctf_descriptor->mutex);
}

static void
generate_metadata (int major, int minor, gchar *UUID, int byte_order)
{
  /* Writing the first sections of the metadata file with the structures 
     and the definitions that will be needed in the future. */

  g_mutex_lock(ctf_descriptor->mutex);
  g_fprintf (ctf_descriptor->metadata, metadata_fmt, major, minor, UUID, byte_order ? "le" : "be");
  g_mutex_unlock(ctf_descriptor->mutex);
}

void
add_metadata_event_struct (const char *metadata_event, gint id,
    gint stream_id)
{
  /* This function only writes the event structure to the metadata file, it
     depends entirely of what is passed as an argument. */
  g_mutex_lock(ctf_descriptor->mutex);
  g_fprintf (ctf_descriptor->metadata, metadata_event, id, stream_id);
  g_mutex_unlock(ctf_descriptor->mutex);
}

void
do_print_cpuusage_event (gint16 event_id, guint32 timestamp,
    guint32 cpunum, guint64 cpuload)
{
  g_mutex_lock(ctf_descriptor->mutex);
  fwrite (&event_id, sizeof (gchar), sizeof (gint16), ctf_descriptor->datastream);
  fwrite (&timestamp, sizeof (gchar), sizeof (guint32), ctf_descriptor->datastream);
  fwrite (&cpunum, sizeof (gchar), sizeof (guint32), ctf_descriptor->datastream);
  fwrite (&cpuload, sizeof (gchar), sizeof (guint64), ctf_descriptor->datastream);
  g_mutex_unlock(ctf_descriptor->mutex);
}

void
do_print_proctime_event (gint16 event_id, guint32 timestamp,
    gchar *elementname, guint64 time)
{
  gint size = strlen (elementname);
  gint pad_num = (size + 1) % 16;
  gchar zero = 0;

  g_mutex_lock(ctf_descriptor->mutex);
  fwrite (&event_id, sizeof (gchar), sizeof (gint16), ctf_descriptor->datastream);
  fwrite (&timestamp, sizeof (gchar), sizeof (guint32), ctf_descriptor->datastream);
  fwrite (elementname, sizeof (gchar), size + 1, ctf_descriptor->datastream);

  /* Verify if padding must be added */
  if (pad_num != 0) {
    pad_num = 16 - pad_num;

    for (; pad_num > 0; --pad_num) {
      fwrite (&zero, sizeof (gchar), 1, ctf_descriptor->datastream);
    }
  }

  fwrite (&time, sizeof (gchar), sizeof (guint64), ctf_descriptor->datastream);
  g_mutex_unlock(ctf_descriptor->mutex);
}

void
do_print_framerate_event (gint16 event_id, guint32 timestamp,
    gchar *padname, guint64 fps)
{
  gint size = strlen (padname);
  gint pad_num = (size + 1) % 16;
  gchar zero = 0;

  g_mutex_lock(ctf_descriptor->mutex);
  fwrite (&event_id, sizeof (gchar), sizeof (gint16), ctf_descriptor->datastream);
  fwrite (&timestamp, sizeof (gchar), sizeof (guint32), ctf_descriptor->datastream);
  fwrite (padname, sizeof (gchar), size + 1, ctf_descriptor->datastream);

  /* Verify if padding must be added */
  if (pad_num != 0) {
    pad_num = 16 - pad_num;

    for (; pad_num > 0; --pad_num) {
      fwrite (&zero, sizeof (gchar), 1, ctf_descriptor->datastream);
    }
  }

  fwrite (&fps, sizeof (gchar), sizeof (guint64), ctf_descriptor->datastream);
  g_mutex_unlock(ctf_descriptor->mutex);
}

void
do_print_interlatency_event (gint16 event_id, guint32 timestamp,
    gchar *originpad, gchar *destinationpad, guint64 time)
{
  gint size = strlen (originpad);
  gint pad_num = (size + 1) % 16;
  gchar zero = 0;

  g_mutex_lock(ctf_descriptor->mutex);
  fwrite (&event_id, sizeof (gchar), sizeof (gint16), ctf_descriptor->datastream);
  fwrite (&timestamp, sizeof (gchar), sizeof (guint32), ctf_descriptor->datastream);
  fwrite (originpad, sizeof (gchar), size + 1, ctf_descriptor->datastream);

  /* Verify if padding must be added */
  if (pad_num != 0) {
    pad_num = 16 - pad_num;

    for (; pad_num > 0; --pad_num) {
      fwrite (&zero, sizeof (gchar), 1, fd);
    }
  }

  size = strlen (destinationpad);
  pad_num = (size + 1) % 16;
  fwrite (destinationpad, sizeof (gchar), size + 1, ctf_descriptor->datastream);

  /* Verify if padding must be added */
  if (pad_num != 0) {
    pad_num = 16 - pad_num;

    for (; pad_num > 0; --pad_num) {
      fwrite (&zero, sizeof (gchar), 1, ctf_descriptor->datastream);
    }
  }

  fwrite (&time, sizeof (gchar), sizeof (uint64_t), ctf_descriptor->datastream);
  g_mutex_unlock(ctf_descriptor->mutex);
}

void
do_print_scheduling_event (gint16 event_id, guint32 timestamp,
    gchar *elementname, guint64 time)
{
  gint size = strlen (elementname);
  gint pad_num = (size + 1) % 16;
  gchar zero = 0;

  g_mutex_lock(ctf_descriptor->mutex);
  fwrite (&event_id, sizeof (gchar), sizeof (gint16), ctf_descriptor->datastream);
  fwrite (&timestamp, sizeof (gchar), sizeof (guint32), ctf_descriptor->datastream);
  fwrite (elementname, sizeof (gchar), size + 1, ctf_descriptor->datastream);

  /* Verify if padding must be added */
  if (pad_num != 0) {
    pad_num = 16 - pad_num;

    for (; pad_num > 0; --pad_num) {
      fwrite (&zero, sizeof (gchar), 1, ctf_descriptor->datastream);
    }
  }

  fwrite (&time, sizeof (gchar), sizeof (guint64), ctf_descriptor->datastream);
  g_mutex_unlock(ctf_descriptor->mutex);
}

void
do_print_init_timer (gint16 event_id, guint32 timestamp,
    guint32 timer)
{
  guint32 unknown = 0;

  g_mutex_lock(ctf_descriptor->mutex);
  fwrite (&event_id, sizeof (gchar), sizeof (gint16), ctf_descriptor->datastream);
  fwrite (&timestamp, sizeof (gchar), sizeof (guint32), ctf_descriptor->datastream);
  fwrite (&unknown, sizeof (gchar), sizeof (guint32), ctf_descriptor->datastream);
  fwrite (&timer, sizeof (gchar), sizeof (guint32), ctf_descriptor->datastream);
  g_mutex_unlock(ctf_descriptor->mutex);
}
