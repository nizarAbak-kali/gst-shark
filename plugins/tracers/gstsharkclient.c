#include <stdlib.h>

#include <string.h>

#include "gstparser.h"
#include "gstctf.h"
#include "gstsharkclient.h"



//~ static trace_information trace_inf_struct;
//~ 
//~ trace_information * trace_inf = &trace_inf_struct;


#define TCP_CONN
void tcp_conn_write(void);
int main (int argc, char * argv[])
{

    //~ const gchar * env_loc_value;
    //~ gchar * env_line;
    //~ gint str_len;
    
    //~ parser_register_callbacks(
        //~ parser_handler_desc_list,
        //~ sizeof(parser_handler_desc_list)/sizeof(parser_handler_desc),
        //~ NULL);
    
    gst_ctf_init ();
    
    //~ env_loc_value = g_getenv ("GST_SHARK_TRACE_LOC");
    //~ 
    //~ if (NULL != env_loc_value)
    //~ {
        //~ str_len = strlen(env_loc_value);
        //~ 
        //~ env_line = g_malloc(str_len + 1);
        //~ 
        //~ strcpy(env_line,env_loc_value);
//~ 
        //~ parser_line(env_line);
        //~ 
        //~ g_free(env_line);
    //~ }
    //~ 
    //~ g_printf("host: %s:%d\n",trace_inf->host_name,trace_inf->port_number);
    //~ g_printf("directory: %s\n",trace_inf->dir_name);
#ifdef TCP_CONN
    tcp_conn_write();
#endif
    gst_ctf_close ();
    
    return EXIT_SUCCESS;
}
