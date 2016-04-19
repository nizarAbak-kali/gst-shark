#include <stdlib.h>
#include <string.h>
#include "gstparser.h"
#include "gstctf.h"
#include "gstsharkclient.h"

#define TCP_CONN

void tcp_conn_write(void);

int main (int argc, char * argv[])
{
    gst_ctf_init ();

#ifdef TCP_CONN
    //~ tcp_conn_write();
#endif
    gst_ctf_close ();

    return EXIT_SUCCESS;
}
