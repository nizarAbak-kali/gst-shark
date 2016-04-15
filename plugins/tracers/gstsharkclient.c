#include <stdlib.h>

#include <string.h>

#include "gstparser.h"
#include "gstsharkclient.h"


/* Default port */
#define SOCKET_PORT     (1000)
#define SOCKET_PROTOCOL G_SOCKET_PROTOCOL_TCP




static trace_information trace_inf_struct;

trace_information * trace_inf = &trace_inf_struct;



static void trace_information_init()
{
    trace_inf->dir_name = NULL;
    trace_inf->host_name = NULL;
    trace_inf->port_number = SOCKET_PORT;
}

static void trace_information_finalize()
{
    if (NULL != trace_inf->dir_name)
    {
        g_free(trace_inf->dir_name);
    }
    if (NULL != trace_inf->host_name)
    {
        g_free(trace_inf->host_name);
    }
    g_object_unref(trace_inf->socket_client);
}


static void tcp_conn_init(void)
{
    GSocketClient * socket_client;
    GSocketConnection * socket_connection;
    GOutputStream * output_stream;
    GError * error;
    /* Creates a new GSocketClient with the default options. */
    socket_client = g_socket_client_new();
    
    g_socket_client_set_protocol (socket_client,SOCKET_PROTOCOL);
    
    /* TODO: see g_socket_client_connect_to_host_async */
    /* Attempts to create a TCP connection to the named host. */
    error = NULL;
    socket_connection = g_socket_client_connect_to_host (socket_client,
                                               trace_inf->host_name,
                                                trace_inf->port_number, /* your port goes here */
                                                NULL,
                                                &error);
    /* Verify connection */
    if (NULL == socket_connection)
    {
        g_object_unref(socket_client);
        trace_inf->conn_output_disable = TRUE;
        return;
    }
    
    output_stream = g_io_stream_get_output_stream (G_IO_STREAM (socket_connection));
    /* Store connection variables */
    trace_inf->socket_client = socket_client;
    trace_inf->socket_connection = socket_connection;
    trace_inf->output_stream = output_stream;
    
    
    
}

#define TCP_CONN

typedef enum {
    FILE_PROTOCOL,
    TCP_PROTOCOL,
    MAX_PROTOCOL
} protocol_type;


const parser_handler_desc parser_handler_desc_list[] = 
{
    {"file://",NULL},
    {"tcp://",NULL},
};

int main (int argc, char * argv[])
{
#ifdef TCP_CONN
    GError * error;
#endif
    const gchar * env_loc_value;
    gchar * env_line;
    gint str_len;
    
    trace_information_init();
    
    parser_register_callbacks(
        parser_handler_desc_list,
        sizeof(parser_handler_desc_list)/sizeof(parser_handler_desc),
        NULL);
    
    
    env_loc_value = g_getenv ("GST_SHARK_TRACE_LOC");
    
    if (NULL != env_loc_value)
    {
        str_len = strlen(env_loc_value);
        
        env_line = g_malloc(str_len + 1);
        
        strcpy(env_line,env_loc_value);

        parse_option(env_line);
        
        g_free(env_line);
    }
    
    g_printf("host: %s\n",trace_inf->host_name);
    g_printf("port: %d\n",trace_inf->port_number);
    g_printf("directory: %s\n",trace_inf->dir_name);
return 0;
    tcp_conn_init();
#ifdef TCP_CONN
    if (TRUE == trace_inf->conn_output_disable)
    {
        return EXIT_SUCCESS;
    }
    g_output_stream_write  (trace_inf->output_stream,
                          "METADATA\n", /* your message goes here */
                          9, /* length of your message */
                          NULL,
                          &error);
                          
    g_output_stream_write  (trace_inf->output_stream,
                          "DATASTREAM\n", /* your message goes here */
                          11, /* length of your message */
                          NULL,
                          &error);
    
    

#endif
    trace_information_finalize();
    
    return EXIT_SUCCESS;
}
