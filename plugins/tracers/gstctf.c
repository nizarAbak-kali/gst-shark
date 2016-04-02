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

#include "gstctf.h"

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

static const char cpuusage_metadata_event[] = "event {\n\
	name = cpuusage;\n\
	id = %d;\n\
	stream_id = %d;\n\
	fields := struct {\n\
		integer { size = 64; align = 8; signed = 0; encoding = none; base = 10; } _cpunum;\n\
		integer { size = 64; align = 8; signed = 0; encoding = none; base = 10; } _cpuload;\n\
	};\n\
};\n\
\n";

static const char proctime_metadata_event[] = "event {\n\
	name = proctime;\n\
	id = %d;\n\
	stream_id = %d;\n\
	fields := struct {\n\
		string _elementname;\n\
		integer { size = 64; align = 8; signed = 0; encoding = none; base = 10; } _time;\n\
	};\n\
};\n\
\n";

static const char framerate_metadata_event[] = "event {\n\
	name = cpuusage;\n\
	id = %d;\n\
	stream_id = %d;\n\
	fields := struct {\n\
		string _padname;\n\
		integer { size = 64; align = 8; signed = 0; encoding = none; base = 10; } _fps;\n\
	};\n\
};\n\
\n";

static const char interlatency_metadata_event[] = "event {\n\
	name = interlatency;\n\
	id = %d;\n\
	stream_id = %d;\n\
	fields := struct {\n\
		string _originpad;\n\
		string _destinationpad;\n\
		integer { size = 64; align = 8; signed = 0; encoding = none; base = 10; } _time;\n\
	};\n\
};\n\
\n";

static const char scheduling_metadata_event[] = "event {\n\
	name = scheduling;\n\
	id = %d;\n\
	stream_id = %d;\n\
	fields := struct {\n\
		string _elementname;\n\
		integer { size = 64; align = 8; signed = 0; encoding = none; base = 10; } _time;\n\
	};\n\
};\n\
\n";

void
generate_data_stream_header (FILE * fd, char *UUID, int UUID_size,
    uint32_t stream_id)
{
  uint64_t time_stamp_begin;
  uint64_t time_stamp_end;
  uint32_t events_discarted;
  uint32_t cpu_id;

  /* The begin of the data stream header is compound by the Magic Number,
     the trace UUID and the Stream ID. These are all required fields. */

  /* Magic Number */
  fwrite (&Magic, sizeof (char), sizeof (Magic), fd);

  /* Trace UUID */
  fwrite (UUID, sizeof (char), UUID_size, fd);

  /* Stream ID */
  fwrite (&stream_id, sizeof (char), sizeof (uint32_t), fd);

  /* The following bytes correspond to the event packet context, these 
     fields are optional. */

  /* Time Stamp begin */
  time_stamp_begin = 0x3e3db41faf8;     // 0xf8fa41dbe3030000
  fwrite (&time_stamp_begin, sizeof (char), sizeof (uint64_t), fd);

  /* Time Stamp end */
  time_stamp_end = 0x000003e3ec8152ee;  // 0xee5281ece3030000;
  fwrite (&time_stamp_end, sizeof (char), sizeof (uint64_t), fd);

  /* Events discarted */
  events_discarted = 0x0;
  fwrite (&events_discarted, sizeof (char), sizeof (uint32_t), fd);

  /* CPU ID */
  cpu_id = 0x0;
  fwrite (&cpu_id, sizeof (char), sizeof (uint32_t), fd);

  /* Padding needed */
  uint32_t unknown = 0x0000FFFF;
  fwrite (&unknown, sizeof (char), sizeof (uint32_t), fd);
}

void
generate_metadata (FILE * fd, int major, int minor, char *UUID, int byte_order)
{
  /* Writing the first sections of the metadata file with the structures 
     and the definitions that will be needed in the future. */

  fprintf (fd, metadata_fmt, major,     /* major */
      minor,                    /* minor */
      UUIDstring, byte_order ? "le" : "be");
}

void
add_metadata_event_struct (FILE * fd, const char *metadata_event, int id,
    int stream_id)
{
  /* This function only writes the event structure to the metadata file, it
     depends entirely of what is passed as an argument. */

  fprint (fd, metadata_event, event_id, stream_id);
}

void
do_print_cpuusage_event (FILE * fd, int16_t event_id, uint32_t timestamp,
    uint32_t cpunum, uint64_t cpuload)
{
  fwrite (&event_id, sizeof (char), sizeof (int16_t), fd);
  fwrite (&timestamp, sizeof (char), sizeof (uint32_t), fd);
  fwrite (&cpunum, sizeof (char), sizeof (uint32_t), fd);
  fwrite (&cpuload, sizeof (char), sizeof (uint64_t), fd);
}

void
do_print_proctime_event (FILE * fd, int16_t event_id, uint32_t timestamp,
    char *elementname, uint64_t time)
{
  int size = strlen (elementname);
  int pad_num = (size + 1) % 16;
  char zero = 0;

  fwrite (&event_id, sizeof (char), sizeof (int16_t), fd);
  fwrite (&timestamp, sizeof (char), sizeof (uint32_t), fd);
  fwrite (elementname, sizeof (char), size + 1, fd);

  /* Verify if padding must be added */
  if (pad_num != 0) {
    pad_num = 16 - pad_num;

    for (; pad_num > 0; --pad_num) {
      fwrite (&zero, sizeof (char), 1, fd);
    }
  }

  fwrite (&time, sizeof (char), sizeof (uint64_t), fd);
}

void
do_print_framerate_event (FILE * fd, int16_t event_id, uint32_t timestamp,
    char *padname, uint64_t fps)
{
  int size = strlen (padname);
  int pad_num = (size + 1) % 16;
  char zero = 0;

  fwrite (&event_id, sizeof (char), sizeof (int16_t), fd);
  fwrite (&timestamp, sizeof (char), sizeof (uint32_t), fd);
  fwrite (padname, sizeof (char), size + 1, fd);

  /* Verify if padding must be added */
  if (pad_num != 0) {
    pad_num = 16 - pad_num;

    for (; pad_num > 0; --pad_num) {
      fwrite (&zero, sizeof (char), 1, fd);
    }
  }

  fwrite (&fps, sizeof (char), sizeof (uint64_t), fd);
}

void
do_print_interlatency_event (FILE * fd, int16_t event_id, uint32_t timestamp,
    char *originpad, char *destinationpad, uint64_t time)
{
  int size = strlen (originpad);
  int pad_num = (size + 1) % 16;
  char zero = 0;

  fwrite (&event_id, sizeof (char), sizeof (int16_t), fd);
  fwrite (&timestamp, sizeof (char), sizeof (uint32_t), fd);
  fwrite (originpad, sizeof (char), size + 1, fd);

  /* Verify if padding must be added */
  if (pad_num != 0) {
    pad_num = 16 - pad_num;

    for (; pad_num > 0; --pad_num) {
      fwrite (&zero, sizeof (char), 1, fd);
    }
  }

  size = strlen (destinationpad);
  pad_num = (size + 1) % 16;
  fwrite (destinationpad, sizeof (char), size + 1, fd);

  /* Verify if padding must be added */
  if (pad_num != 0) {
    pad_num = 16 - pad_num;

    for (; pad_num > 0; --pad_num) {
      fwrite (&zero, sizeof (char), 1, fd);
    }
  }

  fwrite (&time, sizeof (char), sizeof (uint64_t), fd);
}

void
do_print_scheduling_event (FILE * fd, int16_t event_id, uint32_t timestamp,
    char *elementname, uint64_t time)
{
  int size = strlen (elementname);
  int pad_num = (size + 1) % 16;
  char zero = 0;

  fwrite (&event_id, sizeof (char), sizeof (int16_t), fd);
  fwrite (&timestamp, sizeof (char), sizeof (uint32_t), fd);
  fwrite (elementname, sizeof (char), size + 1, fd);

  /* Verify if padding must be added */
  if (pad_num != 0) {
    pad_num = 16 - pad_num;

    for (; pad_num > 0; --pad_num) {
      fwrite (&zero, sizeof (char), 1, fd);
    }
  }

  fwrite (&time, sizeof (char), sizeof (uint64_t), fd);
}

void
do_print_init_timer (FILE * fd, int16_t event_id, uint32_t timestamp,
    uint32_t timer)
{
  uint32_t unknown = 0;

  fwrite (&event_id, sizeof (char), sizeof (int16_t), fd);
  fwrite (&timestamp, sizeof (char), sizeof (uint32_t), fd);
  fwrite (&unknown, sizeof (char), sizeof (uint32_t), fd);
  fwrite (&timer, sizeof (char), sizeof (uint32_t), fd);
}
