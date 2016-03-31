/* gcc -o ctf_app ctf-api-test.c */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define BYTE_ORDER_LE (1)

typedef enum {
    TIMER_INIT_EVENT_ID,
    CPUUSAGE_EVENT_ID,
    PROCTIME_EVENT_ID,
    LATENCY_EVENT_ID,
    FPS_EVENT_ID,
    PIPELINE_GRAPH_EVENT_ID,
    SCHED_TIME_EVENT_ID,
    
} event_id;

int Magic = 0xC1FC1FC1; // 0xc11ffcc1
char UUID[] = {0xd1,0x8e,0x63,0x74,0x35,0xa1,0xcd,0x42,0x8e,0x70,0xa9,0xcf,0xfa,0x71,0x27,0x93};
//~ char UUID[] = {0x67,0xE1,0x89,0xCE,0xA4,0xA0,0x49,0x04,0x83,0x70,0x3B,0xFE,0xFF,0xF3,0x6A,0xDC};
char * UUIDstring = "d18e6374-35a1-cd42-8e70-a9cffa712793";

/* Metadata format string */
static const char metadata_fmt2[] = "\
/* CTF 1.8 */\n\
typealias integer { size = 8; align = 8; signed = false; } := uint8_t;\n\
typealias integer { size = 16; align = 8; signed = false; } := uint16_t;\n\
typealias integer { size = 32; align = 8; signed = false; } := uint32_t;\n\
typealias integer { size = 64; align = 8; signed = false; } := uint64_t;\n\
typealias integer { size = 5; align = 1; signed = false; } := uint5_t;\n\
typealias integer { size = 27; align = 1; signed = false; } := uint27_t;\n\
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
env {\n\
	domain = \"kernel\";\n\
	sysname = \"Linux\";\n\
	kernel_release = \"3.0.0-16-generic-pae\";\n\
	kernel_version = \"#29-Ubuntu SMP Tue Feb 14 13:56:31 UTC 2012\";\n\
	tracer_name = \"lttng-modules\";\n\
	tracer_major = 2;\n\
	tracer_minor = 0;\n\
	tracer_patchlevel = 0;\n\
};\n\
\n\
clock {\n\
	name = monotonic;\n\
	uuid = \"84db105b-b3f4-4821-b662-efc51455106a\";\n\
	description = \"Monotonic Clock\";\n\
	freq = 1000000000; /* Frequency, in Hz */\n\
	/* clock value offset from Epoch is: offset * (1/freq) */\n\
	offset = 1332166405241713987;\n\
};\n\
\n\
typealias integer {\n\
	size = 27; align = 1; signed = false;\n\
	map = clock.monotonic.value;\n\
} := uint27_clock_monotonic_t;\n\
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
struct event_header_compact {\n\
	enum : uint5_t { compact = 0 ... 30, extended = 31 } id;\n\
	variant <id> {\n\
		struct {\n\
			uint27_clock_monotonic_t timestamp;\n\
		} compact;\n\
		struct {\n\
			uint32_t id;\n\
			uint64_clock_monotonic_t timestamp;\n\
		} extended;\n\
	} v;\n\
} align(8);\n\
\n\
struct event_header_large {\n\
	enum : uint16_t { compact = 0 ... 65534, extended = 65535 } id;\n\
	variant <id> {\n\
		struct {\n\
			uint32_clock_monotonic_t timestamp;\n\
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
	event.header := struct event_header_large;\n\
	packet.context := struct packet_context;\n\
};\n\
\n\
event {\n\
	name = sched_stat_runtime;\n\
	id = 37;\n\
	stream_id = 0;\n\
	fields := struct {\n\
		integer { size = 8; align = 8; signed = 1; encoding = UTF8; base = 10; } _comm[16];\n\
		integer { size = 32; align = 8; signed = 1; encoding = none; base = 10; } _tid;\n\
		integer { size = 64; align = 8; signed = 0; encoding = none; base = 10; } _runtime;\n\
		integer { size = 64; align = 8; signed = 0; encoding = none; base = 10; } _vruntime;\n\
	};\n\
};\n\
\n\
event {\n\
	name = sched_switch;\n\
	id = 27;\n\
	stream_id = 0;\n\
	fields := struct {\n\
		integer { size = 8; align = 8; signed = 1; encoding = UTF8; base = 10; } _prev_comm[16];\n\
		integer { size = 32; align = 8; signed = 1; encoding = none; base = 10; } _prev_tid;\n\
		integer { size = 32; align = 8; signed = 1; encoding = none; base = 10; } _prev_prio;\n\
		integer { size = 32; align = 8; signed = 1; encoding = none; base = 10; } _prev_state;\n\
		integer { size = 8; align = 8; signed = 1; encoding = UTF8; base = 10; } _next_comm[16];\n\
		integer { size = 32; align = 8; signed = 1; encoding = none; base = 10; } _next_tid;\n\
		integer { size = 32; align = 8; signed = 1; encoding = none; base = 10; } _next_prio;\n\
	};\n\
};\n\
\n\
event {\n\
	name = timer_start;\n\
	id = 11;\n\
	stream_id = 0;\n\
	fields := struct {\n\
		integer { size = 32; align = 8; signed = 0; encoding = none; base = 10; } _timer;\n\
		integer { size = 32; align = 8; signed = 0; encoding = none; base = 10; } _function;\n\
		integer { size = 32; align = 8; signed = 0; encoding = none; base = 10; } _expires;\n\
		integer { size = 32; align = 8; signed = 0; encoding = none; base = 10; } _now;\n\
	};\n\
};\n\
\n\
event {\n\
	name = timer_init;\n\
	id = 0;\n\
	stream_id = 0;\n\
	fields := struct {\n\
		integer { size = 32; align = 8; signed = 0; encoding = none; base = 10; } _timer;\n\
	};\n\
};\n\
";



static const char metadata_fmt[] =
"/* CTF 1.8 */\n"
"typealias integer { size = 8; align = 8; signed = false; } := uint8_t;\n"
"typealias integer { size = 16; align = 16; signed = false; } := uint16_t;\n"
"typealias integer { size = 32; align = 32; signed = false; } := uint32_t;\n"
"typealias integer { size = 64; align = 64; signed = false; } := uint64_t;\n"
"\n"
"trace {\n"
"	major = %u;\n"			/* major (e.g. 0) */
"	minor = %u;\n"			/* minor (e.g. 1) */
"	uuid = \"%s\";\n"		/* UUID */
"	byte_order = %s;\n"		/* be or le */
"	packet.header := struct {\n"
"		uint32_t magic;\n"
"		uint8_t  uuid[16];\n"
"	};\n"
"};\n"
"\n"
"stream {\n"
"	packet.context := struct {\n"
//~ "		uint64_t content_size;\n"
//~ "		uint64_t packet_size;\n"
"	};\n"
"	typealias integer { size = 64; align = 64; signed = false; } := uint64_t;\n"
"	event.header := struct {\n"
//~ "		enum : uint16_t { compact = 0 ... 65534, extended = 65535 } id;\n"
"		uint64_t timestamp;\n"
"	};\n"					/* Stream event header (opt.) */
"};\n"
"\n";


static const char cpuusage_metadata_event_header[] =
"event {\n\
	name = cpuusage;\n\
	id = %d;\n\
	fields := struct {\n\
		integer { size = 32; align = 8; signed = 0; encoding = none; base = 10; } _num;\n\
		integer { size = 64; align = 8; signed = 0; encoding = none; base = 10; } _usage;\n\
	};\n\
};\n\
\n";


static const char proctime_metadata_event_header[] = "\
event {\n\
        name = proctime;\n\
        id = %d;\n\
        stream_id = 0;\n\
        fields := struct {\n\
                integer { size = 8; align = 8; signed = 1; encoding = UTF8; base = 10; } _element[16];\n\
                integer { size = 64; align = 8; signed = 0; encoding = none; base = 10; } _time;\n\
        };\n\
};\n\
";

static const char fps_metadata_event_header[] = "\
event {\n\
        name = framerate;\n\
        id = %d;\n\
        stream_id = 0;\n\
        fields := struct {\n\
                integer { size = 8; align = 8; signed = 1; encoding = UTF8; base = 10; } _element[16];\n\
                integer { size = 32; align = 8; signed = 0; encoding = none; base = 10; } _fps;\n\
        };\n\
};\n\
";





/*
 * Write event packet header
 */
void CTFDataStreamGenerate(FILE *fd, char * UUID,int UUID_size, uint64_t content_size,uint64_t packet_size )
{
    uint64_t time_stamp_begin;
    uint64_t time_stamp_end;
    uint32_t events_discarted;
    uint32_t cpi_id;
    uint32_t stream_id;
    /* http://diamon.org/ctf/,
     * Section 5. Event packet header
     */
     /* Magic number */
    fwrite(&Magic,sizeof(char),sizeof(Magic),fd);
    /* Trace UUID */
    fwrite(UUID,sizeof(char),UUID_size,fd);
    /* Stream ID (optional) */
    stream_id = 0;
    fwrite(&stream_id,sizeof(char),sizeof(uint32_t),fd);
    /*
     * Section 5. Event packet context
     */
     
    /* Time Stamp begin */
    time_stamp_begin = 0x3e3db41faf8; // 0xf8fa41dbe3030000
    //~ time_stamp_begin = 0xAAAAAAAAAAAAAAAA;
    fwrite(&time_stamp_begin,sizeof(char),sizeof(uint64_t),fd);
    /* Time Stamp end */
    time_stamp_end = 0x000003e3ec8152ee;// 0xee5281ece3030000;
    //~ time_stamp_end = 0xBBBBBBBBBBBBBBBB;
    fwrite(&time_stamp_end,sizeof(char),sizeof(uint64_t),fd);
    /* Events discarted */
    //~ events_discarted = 0xCCCCCCCC;
    events_discarted = 0x0;
    fwrite(&events_discarted,sizeof(char),sizeof(uint32_t),fd);
    
    //~ cpi_id = 0xDDDDDDDD;
    cpi_id = 0x0;
    fwrite(&cpi_id,sizeof(char),sizeof(uint32_t),fd);
    
    /* Content size */
    content_size = 0x000003e800000000;
    //~ fwrite(&content_size,sizeof(char),sizeof(uint64_t),fd);
    /* Packet size */
    packet_size = 0x0004000000000000;  /* 262144 = 0x40000 */
    //~ fwrite(&packet_size,sizeof(char),sizeof(uint64_t),fd);
    /* ? ? ? ? */
    uint32_t unknown = 0x0000FFFF;
    fwrite(&unknown,sizeof(char),sizeof(uint32_t),fd);
}

void CTFDataStreamPading(FILE *fd)
{
    int pad_num = 32640;
    int zero = 0;
    for (;pad_num > 0; --pad_num)
    {
        fwrite(&zero,sizeof(char),1,fd);
    }
}


void CTFMetadataGenerate(FILE *fd, int major, int minor, char * UUID, int byte_order)
{
    /* Convert UUID */
    //~ bt_uuid_unparse(UUID, UUIDstring);
    
    fprintf(fd, metadata_fmt2,
    major, /* major */
    minor, /* minor */
    UUIDstring,
    byte_order ? "le" : "be"
    );
}


void CTFMetadataAddEvent(FILE *fd, const char* metadata_event, event_id id)
{
    fprintf(fd, metadata_event,
    id
    );
}

void CTFNewCpuUsageEvent(FILE *fd, int16_t event_id, uint32_t timestamp, int32_t num, int64_t usage)

{
        //~ int size = strlen(msg);
    /* Add event ID*/
    fwrite(&event_id,sizeof(char),sizeof(int16_t),fd);
    fwrite(&timestamp,sizeof(char),sizeof(uint32_t),fd);
    
    fwrite(&num,sizeof(char),sizeof(uint32_t),fd);
    
    fwrite(&usage,sizeof(char),sizeof(uint64_t),fd);
}


void CTFNewProcTimeEvent(FILE *fd, int16_t event_id, uint32_t timestamp, char * element, int64_t time)
{
    
    fwrite(&event_id,sizeof(char),sizeof(int16_t),fd);

    fwrite(&timestamp,sizeof(char),sizeof(uint32_t),fd);
    
    /*************************************************/
    int size = strlen(element);

    fwrite(element,sizeof(char),size+1,fd);

    /* Verify if padding must be added */
    int pad_num = (size+1)%16;
    char zero=0;
    if (pad_num != 0)
    {
        pad_num = 16 - pad_num;

        for (;pad_num > 0; --pad_num)
        {
            fwrite(&zero,sizeof(char),1,fd);
        }
    }
    /*************************************************/

    fwrite(&time,sizeof(char),sizeof(int64_t),fd);
}

void CTFNewFPSEvent(FILE *fd, int16_t event_id, uint32_t timestamp, char * element, int32_t fps)
{
    
    fwrite(&event_id,sizeof(char),sizeof(int16_t),fd);

    fwrite(&timestamp,sizeof(char),sizeof(uint32_t),fd);
    
    /*************************************************/
    int size = strlen(element);

    fwrite(element,sizeof(char),size+1,fd);

    /* Verify if padding must be added */
    int pad_num = (size+1)%16;
    char zero=0;
    if (pad_num != 0)
    {
        pad_num = 16 - pad_num;

        for (;pad_num > 0; --pad_num)
        {
            fwrite(&zero,sizeof(char),1,fd);
        }
    }
    /*************************************************/

    fwrite(&fps,sizeof(char),sizeof(int32_t),fd);
}


void CTFNewTimerInitEvent(FILE *fd, int16_t event_id, uint32_t timestamp, int32_t timer)
{
    //~ int size = strlen(msg);
    /* Add event ID*/
    int32_t unknown;
    
    fwrite(&event_id,sizeof(char),sizeof(int16_t),fd);

    fwrite(&timestamp,sizeof(char),sizeof(uint32_t),fd);
    
    unknown = 0x000003e3;
    fwrite(&unknown,sizeof(char),sizeof(uint32_t),fd);
    
    fwrite(&timer,sizeof(char),sizeof(uint32_t),fd);

}

void CTFNewTimerStartEvent(FILE *fd, int16_t event_id, uint32_t timestamp, int32_t timer,int32_t function, int32_t expires,int32_t now)
{
    //~ int size = strlen(msg);
    /* Add event ID*/
    fwrite(&event_id,sizeof(char),sizeof(int16_t),fd);
    fwrite(&timestamp,sizeof(char),sizeof(uint32_t),fd);
    
    fwrite(&timer,sizeof(char),sizeof(uint32_t),fd);
    
    fwrite(&function,sizeof(char),sizeof(uint32_t),fd);
    
    fwrite(&expires,sizeof(char),sizeof(uint32_t),fd);
    
    fwrite(&now,sizeof(char),sizeof(uint32_t),fd);
}

void CTFNewSchedStatEvent(FILE *fd, int16_t event_id, uint32_t timestamp,
    char* comm,
    int32_t tid,
    int64_t runtime,
    int64_t vruntime)
{
    //~ int size = strlen(msg);
    /* Add event ID*/
    fwrite(&event_id,sizeof(char),sizeof(int16_t),fd);
    fwrite(&timestamp,sizeof(char),sizeof(uint32_t),fd);
    
    /*************************************************/
    int size = strlen(comm);

    fwrite(comm,sizeof(char),size+1,fd);

    /* Verify if padding must be added */
    int pad_num = (size+1)%16;
    char zero=0;
    if (pad_num != 0)
    {
        pad_num = 16 - pad_num;

        for (;pad_num > 0; --pad_num)
        {
            fwrite(&zero,sizeof(char),1,fd);
        }
    }
    /*************************************************/
    
    fwrite(&tid,sizeof(char),sizeof(uint32_t),fd);
    
    fwrite(&runtime,sizeof(char),sizeof(uint64_t),fd);
    
    fwrite(&vruntime,sizeof(char),sizeof(uint64_t),fd);
}

void CTFNewSchedwitchEvent(FILE *fd, int16_t event_id, uint32_t timestamp,
    char* prev_comm,
    int32_t prev_tid,
    int32_t prev_prio,
    int32_t prev_state,
    char* next_comm,
    int32_t next_tid,
    int32_t next_prio)
{
    //~ int size = strlen(msg);
    /* Add event ID*/
    int size;
    int pad_num;
    char zero;
    
    fwrite(&event_id,sizeof(char),sizeof(int16_t),fd);
    fwrite(&timestamp,sizeof(char),sizeof(uint32_t),fd);
    
    /*************************************************/
    size = strlen(prev_comm);

    fwrite(prev_comm,sizeof(char),size+1,fd);

    /* Verify if padding must be added */
    pad_num = (size+1)%16;
    zero=0;
    if (pad_num != 0)
    {
        pad_num = 16 - pad_num;

        for (;pad_num > 0; --pad_num)
        {
            fwrite(&zero,sizeof(char),1,fd);
        }
    }
    /*************************************************/
    
    fwrite(&prev_tid,sizeof(char),sizeof(uint32_t),fd);
    
    fwrite(&prev_prio,sizeof(char),sizeof(uint32_t),fd);
    
    fwrite(&prev_state,sizeof(char),sizeof(uint32_t),fd);
    
    size = strlen(next_comm);

    fwrite(next_comm,sizeof(char),size+1,fd);

    /* Verify if padding must be added */
    pad_num = (size+1)%16;
    zero=0;
    if (pad_num != 0)
    {
        pad_num = 16 - pad_num;

        for (;pad_num > 0; --pad_num)
        {
            fwrite(&zero,sizeof(char),1,fd);
        }
    }
    
    fwrite(&next_tid,sizeof(char),sizeof(uint32_t),fd);
    
    fwrite(&next_prio,sizeof(char),sizeof(uint32_t),fd);
    
    
}


int main (void)
{
    FILE* fd;
    FILE* FDMetadata;

    /* Create metadata file */
#if 1
    FDMetadata = fopen("metadata", "w");

    CTFMetadataGenerate(FDMetadata, 1, 3, UUID, BYTE_ORDER_LE);
    /* Add event descriptor */
    CTFMetadataAddEvent(FDMetadata,cpuusage_metadata_event_header,CPUUSAGE_EVENT_ID);
    CTFMetadataAddEvent(FDMetadata,proctime_metadata_event_header,PROCTIME_EVENT_ID);
    CTFMetadataAddEvent(FDMetadata,fps_metadata_event_header,FPS_EVENT_ID);

    fclose(FDMetadata);

#endif

    /* Create datastream file */

    fd = fopen("datastream", "w");

    CTFDataStreamGenerate(fd, UUID,sizeof(UUID),0,0);

    /* Add events */
    CTFNewTimerInitEvent(fd, 0, 0xdce73fb4, 3811704140);
    //~ CTFNewTimerStartEvent(fd, 11, 0xdce74686, 3811704140,3238389056,996799,994299);
    //~ 
    //~ CTFNewSchedStatEvent(fd, 37, 0xdce74e29,
    //~ "lttng-sessiond",
    //~ 4072,
    //~ 584455,
    //~ 467336417);
    //~ 
    //~ 
    //~ CTFNewSchedwitchEvent(fd, 27, 0xdce75ece,
    //~ "lttng-sessiond",
    //~ 4072,
    //~ 20,
    //~ 1,
    //~ "swapper",
    //~ 0,
    //~ 20);
    
    CTFNewCpuUsageEvent(fd, CPUUSAGE_EVENT_ID, 10, 0, 11);
    CTFNewCpuUsageEvent(fd, CPUUSAGE_EVENT_ID, 20, 1, 22);
    CTFNewCpuUsageEvent(fd, CPUUSAGE_EVENT_ID, 30, 2, 33);
    CTFNewCpuUsageEvent(fd, CPUUSAGE_EVENT_ID, 40, 3, 44);
    
    
    CTFNewProcTimeEvent(fd, PROCTIME_EVENT_ID, 10, "identity0", 1000);
    CTFNewProcTimeEvent(fd, PROCTIME_EVENT_ID, 20, "queue0", 100);
    CTFNewProcTimeEvent(fd, PROCTIME_EVENT_ID, 30, "identity1", 1500);
    
    CTFNewFPSEvent(fd, FPS_EVENT_ID, 10, "filesrc0", 15);

    fclose(fd);


    return 0;
}
