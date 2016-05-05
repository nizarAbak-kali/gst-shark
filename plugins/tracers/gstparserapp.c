

#include <stdlib.h>
#include <string.h>
#include "gstparser.h"
#include "gstctf.h"

#include <gst/gst.h>
#include <glib.h>
#include <glib/gstdio.h>



struct _GstCtfDescriptor
{
  GstClockTime start_time;
  GMutex mutex;
  //~ guint8 uuid[CTF_UUID_SIZE];
  /* This memory space would be used as auxiliar memory to build the stream
   * that will be written in the FILE or in the socket.
   */
  //~ guint8 mem[CTF_MEM_SIZE];
  /* File variables */
  FILE *metadata;
  FILE *datastream;
  gchar *dir_name;
  gchar *env_dir_name;
  gboolean file_output_disable;

  /* TCP connection variables */
  gchar *host_name;
  gint port_number;
  //~ GSocketClient * socket_client;
  //~ GSocketConnection * socket_connection;
  //~ GOutputStream * output_stream;
  gboolean tcp_output_disable;
};


static GstCtfDescriptor ctf;

static GstCtfDescriptor *ctf_descriptor = &ctf;

static void
tcp_parser_handler (gchar * line)
{
  gchar *line_end;
  gchar *host_name;
  gchar *port_name;
  gchar *port_name_end;
  gsize str_len;

  host_name = line;
  line_end = line;
  while (('\0' != *line_end) && (':' != *line_end)) {
    ++line_end;
  }

  if (*line_end == '\0') {
    str_len = strlen (host_name);

    ctf_descriptor->host_name = g_malloc (str_len + 1);

    strcpy (ctf_descriptor->host_name, host_name);
    /* End of the line, finish parser process */
    return;
  }
  if (*line_end == ':') {
    /* Get the port value */
    *line_end = '\0';

    str_len = strlen (host_name);

    ctf_descriptor->host_name = g_malloc (str_len + 1);

    strcpy (ctf_descriptor->host_name, host_name);

    ++line_end;
    port_name = line_end;

    /* TODO: verify if is a numeric string */
    ctf_descriptor->port_number = g_ascii_strtoull (port_name,
        &port_name_end, 10);

    return;
  }
}

static void
file_parser_handler (gchar * line)
{
  gsize str_len;

  str_len = strlen (line);
  ctf_descriptor->env_dir_name = g_malloc (str_len + 1);
  strcpy (ctf_descriptor->env_dir_name, line);
}

static void
no_match_handler (gchar * line)
{
  gsize str_len;

  str_len = strlen (line);
  ctf_descriptor->env_dir_name = g_malloc (str_len + 1);
  strcpy (ctf_descriptor->env_dir_name, line);
}


static const parser_handler_desc parser_handler_desc_list[] = {
  {"file://", file_parser_handler},
  {"tcp://", tcp_parser_handler},
};


int
main (int argc, char *argv[])
{
  const gchar *env_loc_value;
  gchar *env_line;
  gint str_len;
  GstCTFParser *parser;

  env_loc_value = g_getenv ("GST_SHARK_TRACE_LOC");

  if (NULL != env_loc_value) {

    parser = parser_init ();

    parser_register_callbacks (parser,
        parser_handler_desc_list,
        sizeof (parser_handler_desc_list) / sizeof (parser_handler_desc),
        no_match_handler);

    str_len = strlen (env_loc_value);

    env_line = g_malloc (str_len + 1);

    strcpy (env_line, env_loc_value);

    parser_line (parser, env_line);

    parser_finalize (parser);

    g_free (env_line);
  }

  g_printf ("host: %s:%d\n", ctf_descriptor->host_name,
      ctf_descriptor->port_number);
  g_printf ("directory: %s\n", ctf_descriptor->env_dir_name);

  return EXIT_SUCCESS;
}
