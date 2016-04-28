#include <stdlib.h>
#include <string.h>
#include "gstparser.h"
#include "gstctf.h"
#include "gstsharkclient.h"

static const char cpuusage_metadata_event[] = "event {\n\
	name = cpuusage;\n\
	id = %d;\n\
	stream_id = %d;\n\
	fields := struct {\n\
		integer { size = 32; align = 8; signed = 0; encoding = none; base = 10; } _cpunum;\n\
		integer { size = 64; align = 8; signed = 0; encoding = none; base = 10; } _cpuload;\n\
	};\n\
};\n\
\n";

static const char proctime_metadata_event[] = "event {\n\
	name = proctime;\n\
	id = %d;\n\
	stream_id = %d;\n\
	fields := struct {\n\
        string element; \n\
		integer { size = 64; align = 8; signed = 0; encoding = none; base = 10; } _time;\n\
	};\n\
};\n\
\n";

static const char framerate_metadata_event[] = "event {\n\
	name = framerate;\n\
	id = %d;\n\
	stream_id = %d;\n\
	fields := struct {\n\
		string padname;\n\
		integer { size = 64; align = 8; signed = 0; encoding = none; base = 10; } _fps;\n\
	};\n\
};\n\
\n";

void tcp_conn_write(void);

int main (int argc, char * argv[])
{
    gchar *metadata_event;
    
    gst_ctf_init ();
    
    
    metadata_event =
    g_strdup_printf (cpuusage_metadata_event, CPUUSAGE_EVENT_ID, 0);
    add_metadata_event_struct (metadata_event);
    g_free (metadata_event);
    
    metadata_event =
    g_strdup_printf (proctime_metadata_event, PROCTIME_EVENT_ID, 0);
    add_metadata_event_struct (metadata_event);
    g_free (metadata_event);
    
    metadata_event =
    g_strdup_printf (framerate_metadata_event, FPS_EVENT_ID, 0);
    add_metadata_event_struct (metadata_event);
    g_free (metadata_event);

    /* Events */
    do_print_cpuusage_event (CPUUSAGE_EVENT_ID, 0, 75);
    
    do_print_proctime_event (PROCTIME_EVENT_ID, "identity0", 128);
    
    do_print_framerate_event (FPS_EVENT_ID, "identity0_pad", 256);

    gst_ctf_close ();

    return EXIT_SUCCESS;
}
