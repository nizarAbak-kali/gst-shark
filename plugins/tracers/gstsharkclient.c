#include <stdlib.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <string.h>


#define SOCKET_PORT (1000)

typedef struct
{
    gchar * host_name;
    gchar * dir_name;
    gint port_number;
} trace_information;


typedef enum {
    FILE_PROTOCOL,
    TCP_PROTOCOL,
    MAX_PROTOCOL
} protocol_type;

static trace_information trace_inf_struct;

static trace_information * trace_inf = &trace_inf_struct;

static gchar * protocol_list[] = {
    [FILE_PROTOCOL] = "file://",
    [TCP_PROTOCOL]  = "tcp://",
};


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
}

static gboolean parse_strcmp(const gchar * ref, gchar ** cmp_string)
{
    gchar* string;
    
    string = *cmp_string;
    
    while (*ref == *string && '\0' != *ref)
    {
        ref++;
        string++;
    }
    /* Verify if the loop reaches the null character */
    if ('\0' == *ref)
    {
        *cmp_string = string;
        return TRUE;
    }
    return FALSE;
}

static gboolean parser_get_protocol(protocol_type * type, gchar ** line)
{
    gint protocol_type_idx;
    gboolean cmp_res;
    gchar* string = *line;
    
    for (protocol_type_idx = 0; protocol_type_idx < MAX_PROTOCOL; ++protocol_type_idx)
    {
        cmp_res = parse_strcmp(protocol_list[protocol_type_idx],&string);
        if (TRUE == cmp_res)
        {
            *line = string;
            *type = protocol_type_idx;
            return TRUE;
        }
    }
    return FALSE;
}
    
    

static void parse_option(gchar * option)
{

    protocol_type type;
    gboolean parser_prot_res;
    //~ gchar * env = "file:///tmp/gst-shark;tcp://10.251.101.126:5555";
    gchar * env = option;
    gchar * line = env;
    gchar * dir_name;
    gchar * line_end;
    gchar * host_name;
    gchar * port_name;
    gboolean end_of_line = FALSE;
    guint str_len;
    
    do
    {
        parser_prot_res = parser_get_protocol(&type, &line);
        
        if(FALSE == parser_prot_res)
        {
            /* TODO */
        }

        switch (type)
        {
            case FILE_PROTOCOL:
                dir_name = line;
                line_end = line;
                
                while (('\0' != *line_end) && (';' != *line_end))
                {
                    ++line_end;
                }

                if (*line_end == '\0')
                {
                    str_len = strlen(dir_name);
                    
                    trace_inf->dir_name = g_malloc(str_len + 1);
                    
                    strcpy(trace_inf->dir_name,dir_name);
                    /* End of the line, finish parser process */
                    end_of_line = TRUE;
                    break;
                }
                *line_end = '\0';
                str_len = strlen(dir_name);
                    
                trace_inf->dir_name = g_malloc(str_len + 1);
                    
                strcpy(trace_inf->dir_name,dir_name);
                line = line_end + 1;
                
                break;
            case TCP_PROTOCOL:
                host_name = line;
                line_end = line;
                while (('\0' != *line_end) && 
                    (':' != *line_end) &&
                    (';' != *line_end))
                {
                    ++line_end;
                }
                
                if (*line_end == '\0')
                {
                    str_len = strlen(host_name);
                    
                    trace_inf->host_name = g_malloc(str_len + 1);
                    
                    strcpy(trace_inf->host_name,host_name);
                    /* End of the line, finish parser process */
                    end_of_line = TRUE;
                    break;
                }
                if (*line_end == ':')
                {
                    /* Get the port value */
                    *line_end = '\0';
                    
                    str_len = strlen(host_name);
                    
                    trace_inf->host_name = g_malloc(str_len + 1);
                    
                    strcpy(trace_inf->host_name,host_name);
                    
                    ++line_end;
                    port_name = line_end;
                    while (('\0' != *line_end) && (';' != *line_end))
                    {
                        ++line_end;
                    }
                    
                    if (*line_end == '\0')
                    {
                        /* TODO: verify if is a numeric string */
                        trace_inf->port_number = atoi(port_name);
                        /* End of the line, finish parser process */
                        end_of_line = TRUE;
                        break;
                    }
                    /* if *line_end == ';' */
                    *line_end = '\0';
                    trace_inf->port_number = atoi(port_name);
                    line = line_end + 1;
                }
                /* if *line_end == ';' */
				*line_end = '\0';

				str_len = strlen(host_name);
				trace_inf->host_name = g_malloc(str_len + 1);
				strcpy(trace_inf->host_name,host_name);
				line = line_end + 1;
                break;
            default:
                break;
        }
    } while (FALSE == end_of_line);
}

//~ #define TCP_CONN


int main (int argc, char * argv[])
{
#ifdef TCP_CONN
    GSocketClient * socket_client;
    GSocketConnection * socket_connection;
    GInputStream * input_stream;
    GOutputStream * output_stream;
    GError * error;
#endif
    const gchar * env_loc_value;
    gchar * env_line;
    gint str_len;
    
    trace_information_init();
    
    
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
    
    
#ifdef TCP_CONN
    
    /* Creates a new GSocketClient with the default options. */
    socket_client = g_socket_client_new();
    
    /* TODO: 
     * void  g_socket_client_set_protocol (GSocketClient *client,
     *                               GSocketProtocol protocol);
     */
    
    /* TODO: see g_socket_client_connect_to_host_async */
    /* Attempts to create a TCP connection to the named host. */
    socket_connection = g_socket_client_connect_to_host (socket_client,
                                               (gchar*)"localhost",
                                                80, /* your port goes here */
                                                NULL,
                                                &error);
    
    input_stream = g_io_stream_get_input_stream (G_IO_STREAM (socket_connection));
    output_stream = g_io_stream_get_output_stream (G_IO_STREAM (socket_connection));
    g_output_stream_write  (output_stream,
                          "Hello server!\n", /* your message goes here */
                          14, /* length of your message */
                          NULL,
                          &error);
                          
        g_output_stream_write  (output_stream,
                          "Hello server!\n", /* your message goes here */
                          14, /* length of your message */
                          NULL,
                          &error);
  
                                                
    if (NULL == socket_connection)
    {
        /*ERROR*/
    }
    
    
    g_object_unref(socket_client);
#endif
    trace_information_finalize();
    
    return EXIT_SUCCESS;
}
