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
#include <glib/gprintf.h>
#include <gio/gio.h>
#include <string.h>
#include <stdlib.h> /* TODO: remove atoi */

#include "gstctf.h"
#include "gstparser.h"

#define MAX_DIRNAME_LEN (30)

/* Default port */
#define SOCKET_PORT     (1000)
#define SOCKET_PROTOCOL G_SOCKET_PROTOCOL_TCP
#define CTF_MEM_SIZE      (2024)
#define CTF_UUID_SIZE     (16)

static void file_parser_handler(gchar * line);
static void tcp_parser_handler(gchar * line);

typedef enum
{
  BYTE_ORDER_BE,
  BYTE_ORDER_LE,
} byte_order;

struct _GstCtfDescriptor
{
  GstClockTime start_time;
  GMutex mutex;
  guint8 uuid[CTF_UUID_SIZE];
  /* This memory space would be used as auxiliar memory to build the stream
   * that will be written in the FILE or in the socket.
   */
  guint8 mem[CTF_MEM_SIZE];
  /* File variables */
  FILE *metadata;
  FILE *datastream;
  gchar *dir_name;
  gchar *env_dir_name;
  gboolean file_output_disable;

  /* TCP connection variables */
  gchar* host_name;
  gint port_number;
  GSocketClient * socket_client;
  GSocketConnection * socket_connection;
  GOutputStream * output_stream;
  gboolean tcp_output_disable;
};

static GstCtfDescriptor *ctf_descriptor = NULL;

static const parser_handler_desc parser_handler_desc_list[] =
{
    {"file://",file_parser_handler},
    {"tcp://",tcp_parser_handler},
};

/* Metadata format string */
static const char metadata_fmt[] = "\
/* CTF 1.8 */\n\
typealias integer { size = 8; align = 8; signed = false; } := uint8_t;\n\
typealias integer { size = 16; align = 8; signed = false; } := uint16_t;\n\
typealias integer { size = 32; align = 8; signed = false; } := uint32_t;\n\
typealias integer { size = 64; align = 8; signed = false; } := uint64_t;\n\
\n\
trace {\n\
	major = %u;\n\
	minor = %u;\n\
	uuid = \"%s\";\n\
	byte_order = %s;\n\
	packet.header := struct {\n\
		uint32_t magic;\n\
		uint8_t  uuid[16];\n\
		uint32_t stream_id;\n\
	};\n\
};\n\
\n\
clock { \n\
	name = monotonic; \n\
	uuid = \"84db105b-b3f4-4821-b662-efc51455106a\"; \n\
	description = \"Monotonic Clock\"; \n\
	freq = 1000000; /* Frequency, in Hz */ \n\
	/* clock value offset from Epoch is: offset * (1/freq) */ \n\
    offset_s = 21600; \n\
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
};\n\
\n\
struct event_header {\n\
	enum : uint16_t { compact = 0 ... 65534, extended = 65535 } id;\n\
	variant <id> {\n\
		struct {\n\
			uint32_t timestamp;\n\
		} compact;\n\
		struct {\n\
			uint32_t id;\n\
			uint64_t timestamp;\n\
		} extended;\n\
	} v;\n\
} align(8);\n\
\n\
stream {\n\
	id = 0;\n\
	event.header := struct event_header;\n\
	packet.context := struct packet_context;\n\
};\n\
\n\
event {\n\
	name = init;\n\
	id = 0;\n\
	stream_id = 0;\n\
};\n\
";


GstCtfDescriptor * ctf_create_struct()
{
    gchar UUID[] =
      { 0xd1, 0x8e, 0x63, 0x74, 0x35, 0xa1, 0xcd, 0x42, 0x8e, 0x70, 0xa9, 0xcf,
    0xfa, 0x71, 0x27, 0x93
    };
    GstCtfDescriptor * ctf;

    ctf = g_malloc (sizeof (GstCtfDescriptor));
    if (NULL == ctf)
    {
        GST_ERROR ("CTF descriptor could not be created.");
        return NULL;
    }

    /* File variables */
    ctf->dir_name = NULL;
    ctf->env_dir_name = NULL;
    /* Default state Enable */
    ctf->file_output_disable = FALSE;

    ctf->metadata = NULL;
    ctf->datastream = NULL;

    /* TCP connection variables */
    ctf->host_name = NULL;
    ctf->port_number = SOCKET_PORT;

    ctf->socket_client = NULL;
    ctf->socket_connection = NULL;
    ctf->output_stream = NULL;

    /* Default TCP connection state Enable */
    ctf->tcp_output_disable = FALSE;

    /* Currently a constant UUID value is used */
    memcpy(ctf->uuid,UUID,CTF_UUID_SIZE);

    return ctf;
}

static void
generate_datastream_header ()
{
  guint64 time_stamp_begin;
  guint64 time_stamp_end;
  guint32 Magic = 0xC1FC1FC1;
  guint32 unknown;
  gint32 stream_id;
  guint8 *mem;
  guint data_len;
  GError * error;

  stream_id = 0;

  data_len = CTF_UUID_SIZE + 4 + 8 + 8 + 4 + 4;
  /* Create Stream */
  mem = ctf_descriptor->mem;

  g_mutex_lock (&ctf_descriptor->mutex);
  /* The begin of the data stream header is compound by the Magic Number,
     the trace UUID and the Stream ID. These are all required fields. */
  /* Magic Number */
  *(guint32*)mem = Magic;
  mem += sizeof(guint32);
  /* Trace UUID */
  memcpy(mem,ctf_descriptor->uuid,CTF_UUID_SIZE);
  mem += CTF_UUID_SIZE;
  /* Stream ID */
  *(guint32*)mem = stream_id;
  mem += sizeof(guint32);

  /* Time Stamp begin */
  time_stamp_begin = 0;
  *(guint64*)mem = time_stamp_begin;
  mem += sizeof(guint64);

  /* Time Stamp end */
  time_stamp_end = 0;
  *(guint64*)mem = time_stamp_end;
  mem += sizeof(guint64);

  /* Padding needed */
  unknown = 0x0000FFFF;
  *(guint32*)mem = unknown;
  mem += sizeof(guint32);

  fwrite (ctf_descriptor->mem, sizeof (gchar), data_len , ctf_descriptor->datastream);

  if (FALSE == ctf_descriptor->tcp_output_disable )
  {
    g_output_stream_write  (ctf_descriptor->output_stream,
                          ctf_descriptor->mem,
                          data_len,
                          NULL,
                          &error);
  }

  g_mutex_unlock (&ctf_descriptor->mutex);
}

static void
uuid_to_uuidstring (gchar * uuid_string, guint8 * uuid)
{
  gchar *uuid_string_idx;
  gint32 byte;
  gint uuid_idx;

  uuid_string_idx = uuid_string;
  uuid_idx = 0;
  for (uuid_idx = 0; uuid_idx < 4; ++uuid_idx) {
    byte = 0xFF & uuid[uuid_idx];

    g_sprintf (uuid_string_idx, "%x", byte);
    uuid_string_idx += 2;
  }
  *(uuid_string_idx++) = '-';

  for (; uuid_idx < 6; ++uuid_idx) {
    byte = 0xFF & uuid[uuid_idx];
    g_sprintf (uuid_string_idx, "%x", byte);
    uuid_string_idx += 2;
  }
  *(uuid_string_idx++) = '-';

  for (; uuid_idx < 8; ++uuid_idx) {
    byte = 0xFF & uuid[uuid_idx];

    g_sprintf (uuid_string_idx, "%x", byte);
    uuid_string_idx += 2;
  }
  *(uuid_string_idx++) = '-';

  for (; uuid_idx < 10; ++uuid_idx) {
    byte = 0xFF & uuid[uuid_idx];
    g_sprintf (uuid_string_idx, "%x", byte);
    uuid_string_idx += 2;
  }
  *(uuid_string_idx++) = '-';

  for (; uuid_idx < 16; ++uuid_idx) {
    byte = 0xFF & uuid[uuid_idx];
    g_sprintf (uuid_string_idx, "%x", byte);
    uuid_string_idx += 2;
  }

  *++uuid_string_idx = 0;
}

static void
generate_metadata (gint major, gint minor, gint byte_order)
{
    gint str_len;
    GError * error;
  /* Writing the first sections of the metadata file with the structures
     and the definitions that will be needed in the future. */

  gchar uuid_string[] = "XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX0";
  uuid_to_uuidstring (uuid_string, ctf_descriptor->uuid);

  str_len = g_snprintf((gchar*)ctf_descriptor->mem,CTF_MEM_SIZE ,metadata_fmt,major, minor, uuid_string, byte_order ? "le" : "be");
  if (CTF_MEM_SIZE == str_len)
  {
    GST_ERROR ("Insufficient memory to create metadata");
    return;
  }

  g_mutex_lock (&ctf_descriptor->mutex);

  fwrite (ctf_descriptor->mem, sizeof (gchar), str_len, ctf_descriptor->metadata);

  if (FALSE == ctf_descriptor->tcp_output_disable )
  {
    g_output_stream_write  (ctf_descriptor->output_stream,
                          ctf_descriptor->mem,
                          str_len,
                          NULL,
                          &error);
  }
  g_mutex_unlock (&ctf_descriptor->mutex);
}

gchar *
get_ctf_path_name (void)
{
  return ctf_descriptor->dir_name;
}

static void tcp_parser_handler(gchar * line)
{
    gchar * line_end;
    gchar * host_name;
    gchar * port_name;
    gsize str_len;

    host_name = line;
    line_end = line;
    while (('\0' != *line_end) &&
        (':' != *line_end))
    {
        ++line_end;
    }

    if (*line_end == '\0')
    {
        str_len = strlen(host_name);

        ctf_descriptor->host_name = g_malloc(str_len + 1);

        strcpy(ctf_descriptor->host_name,host_name);
        /* End of the line, finish parser process */
        return;
    }
    if (*line_end == ':')
    {
        /* Get the port value */
        *line_end = '\0';

        str_len = strlen(host_name);

        ctf_descriptor->host_name = g_malloc(str_len + 1);

        strcpy(ctf_descriptor->host_name,host_name);

        ++line_end;
        port_name = line_end;

        /* TODO: verify if is a numeric string */
        ctf_descriptor->port_number = atoi(port_name);

        return;

    }
}

static void file_parser_handler(gchar * line)
{
    gsize  str_len;

    str_len = strlen(line);
    ctf_descriptor->env_dir_name = g_malloc(str_len + 1);
    strcpy(ctf_descriptor->env_dir_name,line);
}


static void
ctf_process_env_var()
{
  const gchar * env_loc_value;
  gchar dir_name[30];
  gchar *env_dir_name;
  gchar * env_line;
  gint size_env_path = 0;
  gint str_len;
  time_t now = time (NULL);

  env_loc_value = g_getenv ("GST_SHARK_TRACE_LOC");

  if (NULL != env_loc_value)
  {
     parser_register_callbacks(
       parser_handler_desc_list,
       sizeof(parser_handler_desc_list)/sizeof(parser_handler_desc),
       NULL);

     str_len = strlen(env_loc_value);

     env_line = g_malloc(str_len + 1);

     strcpy(env_line,env_loc_value);

     parser_line(env_line);

     g_free(env_line);
  }

  g_printf("host: %s:%d\n",ctf_descriptor->host_name,ctf_descriptor->port_number);
  g_printf("directory: %s\n",ctf_descriptor->env_dir_name);

  if (G_UNLIKELY (g_getenv ("GST_SHARK_CTF_DISABLE") != NULL)) {
    env_dir_name = (gchar *) g_getenv ("PWD");
    ctf_descriptor->file_output_disable = TRUE;
  } else {
    env_dir_name = ctf_descriptor->env_dir_name;
  }

  if (G_LIKELY (env_dir_name == NULL)) {
    /* Creating the output folder for the CTF output files. */
    strftime (dir_name, MAX_DIRNAME_LEN, "gstshark_%Y%m%d%H%M%S",
        localtime (&now));
    ctf_descriptor->dir_name = g_malloc (MAX_DIRNAME_LEN + 1);
    g_stpcpy (ctf_descriptor->dir_name, dir_name);
  } else {
    size_env_path = strlen (env_dir_name);
    ctf_descriptor->dir_name = g_malloc (size_env_path + 1);
    g_stpcpy (ctf_descriptor->dir_name, env_dir_name);
  }
}


static void
create_ctf_path (gchar * dir_name)
{
  g_return_if_fail (dir_name);

  if (!g_file_test (dir_name, G_FILE_TEST_EXISTS)) {
    if (g_mkdir (dir_name, 0775) == 0) {
      GST_INFO ("Directory %s did not exist and was created sucessfully.",
          dir_name);
    } else {
      GST_ERROR ("Directory %s could not be created.", dir_name);
    }
  } else {
    GST_INFO ("Directory %s already exists in the current path.", dir_name);
  }
}

void ctf_file_init()
{
  gchar *metadata_file;
  gchar *datastream_file;

    if ( TRUE != ctf_descriptor->file_output_disable)
    {
        /* Creating the output folder for the CTF output files. */
        create_ctf_path (ctf_descriptor->dir_name);

        datastream_file =
            g_strjoin (G_DIR_SEPARATOR_S, ctf_descriptor->dir_name, "datastream", NULL);
        metadata_file =
            g_strjoin (G_DIR_SEPARATOR_S, ctf_descriptor->dir_name, "metadata", NULL);

        ctf_descriptor->datastream = g_fopen (datastream_file, "w");
        if (ctf_descriptor->datastream == NULL) {
            GST_ERROR ("Could not open datastream file, path does not exist.");
        }

        ctf_descriptor->metadata = g_fopen (metadata_file, "w");
        if (ctf_descriptor->metadata == NULL) {
            GST_ERROR ("Could not open metadata file, path does not exist.");
        }

        g_mutex_init (&ctf_descriptor->mutex);
        ctf_descriptor->start_time = gst_util_get_timestamp ();
        ctf_descriptor->file_output_disable = FALSE;

        g_free (datastream_file);
        g_free (metadata_file);
    }
}

static void ctf_tcp_init(void)
{
    GSocketClient * socket_client;
    GSocketConnection * socket_connection;
    GOutputStream * output_stream;
    GError * error;
    /* Verify if the host name was given */
    if (NULL == ctf_descriptor->host_name)
    {
        ctf_descriptor->tcp_output_disable = TRUE;
        return;
    }

    /* Creates a new GSocketClient with the default options. */
    socket_client = g_socket_client_new();

    g_socket_client_set_protocol (socket_client,SOCKET_PROTOCOL);

    /* TODO: see g_socket_client_connect_to_host_async */
    /* Attempts to create a TCP connection to the named host. */
    error = NULL;
    socket_connection = g_socket_client_connect_to_host (socket_client,
                                               ctf_descriptor->host_name,
                                               ctf_descriptor->port_number, /* your port goes here */
                                               NULL,
                                               &error);
    /* Verify connection */
    if (NULL == socket_connection)
    {
        g_object_unref(socket_client);
        socket_client = NULL;
        ctf_descriptor->tcp_output_disable = TRUE;
        return;
    }

    output_stream = g_io_stream_get_output_stream (G_IO_STREAM (socket_connection));
    /* Store connection variables */
    ctf_descriptor->socket_client = socket_client;
    ctf_descriptor->socket_connection = socket_connection;
    ctf_descriptor->output_stream = output_stream;
}

gboolean
gst_ctf_init (void)
{
  if (ctf_descriptor) {
    GST_ERROR ("CTF Descriptor already exists.");
    return FALSE;
  }

  /* Since the descriptors structure does not exist it is needed to
     create and initialize a new one. */
  ctf_descriptor = ctf_create_struct();
  /* Load and proccess enviroment variables */
  ctf_process_env_var();

  ctf_file_init();
  ctf_tcp_init();

  if (!ctf_descriptor->file_output_disable) {
    generate_metadata (1, 3, BYTE_ORDER_LE);
    generate_datastream_header ();
    do_print_ctf_init (INIT_EVENT_ID);
  }

  return TRUE;
}

void
add_metadata_event_struct (const gchar * metadata_event)
{
  /* This function only writes the event structure to the metadata file, it
     depends entirely of what is passed as an argument. */

  if (ctf_descriptor->file_output_disable)
    return;

  g_mutex_lock (&ctf_descriptor->mutex);
  g_fprintf (ctf_descriptor->metadata, "%s", metadata_event);
  g_mutex_unlock (&ctf_descriptor->mutex);
}

static void
add_event_header (event_id id)
{
  guint32 timestamp;
  guint8 * mem;
  guint data_size;
  GError * error;
  
  GstClockTime elapsed =
      GST_CLOCK_DIFF (ctf_descriptor->start_time, gst_util_get_timestamp ());

  mem = ctf_descriptor->mem;

  elapsed = elapsed / 1000;
  timestamp = elapsed;
  /* Add event ID */
  data_size = 6;

  /* Write event ID */
  *(guint16*)mem = id;
  mem += sizeof(guint16);
  /* Write timestamp */
  *(guint32*)mem = timestamp;
  mem += sizeof(guint32);


  if (FALSE == ctf_descriptor->file_output_disable )
  {
    fwrite (ctf_descriptor->mem, sizeof (gchar), data_size, ctf_descriptor->datastream);
  }

  if (FALSE == ctf_descriptor->tcp_output_disable )
  {
  g_output_stream_write  (ctf_descriptor->output_stream,
                          ctf_descriptor->mem,
                          data_size,
                          NULL,
                          &error);
  }
}

void
do_print_cpuusage_event (event_id id, guint32 cpunum, guint64 cpuload)
{
  if (ctf_descriptor->file_output_disable)
    return;

  g_mutex_lock (&ctf_descriptor->mutex);
  add_event_header (id);
  fwrite (&cpunum, sizeof (gchar), sizeof (guint32),
      ctf_descriptor->datastream);
  fwrite (&cpuload, sizeof (gchar), sizeof (guint64),
      ctf_descriptor->datastream);
  g_mutex_unlock (&ctf_descriptor->mutex);
}


void
do_print_proctime_event (event_id id, gchar * elementname, guint64 time)
{
  gint size = strlen (elementname);

  if (ctf_descriptor->file_output_disable)
    return;

  g_mutex_lock (&ctf_descriptor->mutex);
  add_event_header (id);
  fwrite (elementname, sizeof (gchar), size + 1, ctf_descriptor->datastream);
  fwrite (&time, sizeof (gchar), sizeof (guint64), ctf_descriptor->datastream);
  g_mutex_unlock (&ctf_descriptor->mutex);
}

void
do_print_framerate_event (event_id id, const gchar * padname, guint64 fps)
{
  gint size = strlen (padname);

  if (ctf_descriptor->file_output_disable)
    return;

  g_mutex_lock (&ctf_descriptor->mutex);
  add_event_header (id);
  fwrite (padname, sizeof (gchar), size + 1, ctf_descriptor->datastream);
  fwrite (&fps, sizeof (gchar), sizeof (guint64), ctf_descriptor->datastream);
  g_mutex_unlock (&ctf_descriptor->mutex);
}

void
do_print_interlatency_event (event_id id,
    gchar * originpad, gchar * destinationpad, guint64 time)
{
  gint size = strlen (originpad);

  if (ctf_descriptor->file_output_disable)
    return;

  g_mutex_lock (&ctf_descriptor->mutex);
  add_event_header (id);
  fwrite (originpad, sizeof (gchar), size + 1, ctf_descriptor->datastream);

  size = strlen (destinationpad);
  fwrite (destinationpad, sizeof (gchar), size + 1, ctf_descriptor->datastream);

  fwrite (&time, sizeof (gchar), sizeof (guint64), ctf_descriptor->datastream);
  g_mutex_unlock (&ctf_descriptor->mutex);
}

void
do_print_scheduling_event (event_id id, gchar * elementname, guint64 time)
{
  gint size = strlen (elementname);

  if (ctf_descriptor->file_output_disable)
    return;

  g_mutex_lock (&ctf_descriptor->mutex);
  add_event_header (id);
  fwrite (elementname, sizeof (gchar), size + 1, ctf_descriptor->datastream);
  fwrite (&time, sizeof (gchar), sizeof (guint64), ctf_descriptor->datastream);
  g_mutex_unlock (&ctf_descriptor->mutex);
}

void
do_print_ctf_init (event_id id)
{
  GError * error;
  guint32 unknown = 0;

  g_mutex_lock (&ctf_descriptor->mutex);
  
  add_event_header (id);
  
  if (FALSE == ctf_descriptor->file_output_disable )
  {
    fwrite (&unknown, sizeof (gchar), sizeof (guint32),
      ctf_descriptor->datastream);
  }
      
  if (FALSE == ctf_descriptor->tcp_output_disable )
  {
    g_output_stream_write  (ctf_descriptor->output_stream,
                          &unknown,
                          sizeof (guint32),
                          NULL,
                          &error);
  }
      
      
      
  g_mutex_unlock (&ctf_descriptor->mutex);
}

void
gst_ctf_close (void)
{
  fclose (ctf_descriptor->metadata);
  fclose (ctf_descriptor->datastream);
  g_mutex_clear (&ctf_descriptor->mutex);

  if (NULL != ctf_descriptor->dir_name)
  {
      g_free(ctf_descriptor->dir_name);
  }
  if (NULL != ctf_descriptor->host_name)
  {
      g_free(ctf_descriptor->host_name);
  }
  if (NULL != ctf_descriptor->socket_client)
  {
    g_object_unref(ctf_descriptor->socket_client);
  }

  g_free (ctf_descriptor);
}


/* Only for test */
void tcp_conn_write(void)
{
    GError * error;

    if (TRUE == ctf_descriptor->tcp_output_disable )
    {
        return;
    }

    g_output_stream_write  (ctf_descriptor->output_stream,
                          "METADATA\n", /* your message goes here */
                          9, /* length of your message */
                          NULL,
                          &error);

    g_output_stream_write  (ctf_descriptor->output_stream,
                          "DATASTREAM\n", /* your message goes here */
                          11, /* length of your message */
                          NULL,
                          &error);
}
