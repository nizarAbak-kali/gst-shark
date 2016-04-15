
#ifndef __GSTSHARKCLIENT_H__
#define __GSTSHARKCLIENT_H__

#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>

typedef struct
{
    gchar * host_name;
    gchar * dir_name;
    gint port_number;
    
    GSocketClient * socket_client;
    GSocketConnection * socket_connection;
    //~ GInputStream * input_stream;
    GOutputStream * output_stream;
    
    gboolean conn_output_disable;
} trace_information;

#endif /* __GSTSHARKCLIENT_H__ */
