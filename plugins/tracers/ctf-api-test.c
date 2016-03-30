/* gcc -o ctf_app ctf-api-test.c */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define BYTE_ORDER_LE (1)

typedef enum {
    CPUUSAGE_EVENT_ID,
} event_id;

int Magic = 0xC1FC1FC1;

char UUID[] = {0x67,0xE1,0x89,0xCE,0xA4,0xA0,0x49,0x04,0x83,0x70,0x3B,0xFE,0xFF,0xF3,0x6A,0xDC};
char * UUIDstring = "67e189ce-a4a0-4904-8370-3bfefff36adc";

/* Metadata format string */
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
"event {\n"
"	name = cpuusage;\n"
"	id = %d;\n"
"	fields := struct { string str; };\n"
"};\n"
"\n";


static const char proctime_metadata_event_header[] =
"event {\n"
"	name = proctime;\n"
"	id = %d;\n"
"	fields := struct { string str; };\n"
"};\n"
"\n";



/*
 * Write event packet header
 */
void CTFDataStreamGenerate(FILE *fd, char * UUID,int UUID_size, uint64_t content_size,uint64_t packet_size )
{
    /* http://diamon.org/ctf/,
     * Section 5. Event packet header
     */
     /* Magic number */
    fwrite(&Magic,sizeof(char),sizeof(Magic),fd);
    /* Trace UUID */
    fwrite(UUID,sizeof(char),UUID_size,fd);
    /* Stream ID (optional) */

    /*
     * Section 5. Event packet context
     */

    /* Content size */
    content_size = 0x000003e800000000;
    //~ fwrite(&content_size,sizeof(char),sizeof(uint64_t),fd);
    /* Packet size */
    packet_size = 0x0004000000000000;  /* 262144 = 0x40000 */
    //~ fwrite(&packet_size,sizeof(char),sizeof(uint64_t),fd);
    /* ? ? ? ? */
    uint32_t unknown = 0;
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

    fprintf(fd, metadata_fmt,
    major, /* major */
    minor, /* minor */
    UUIDstring,
    byte_order ? "le" : "be"
    );
}


void CTFMetadataAddEvent(FILE *fd, const char* metadata_event, int event_id)
{
    fprintf(fd, metadata_event,
    event_id
    );
}

void CTFNewCpuUsageEvent(FILE *fd, int16_t event_id, uint64_t timestamp,
//~ int cpu_num, double usage)
char * msg)
{
    int size = strlen(msg);
    /* Add event ID*/
    //~ fwrite(&event_id,sizeof(char),sizeof(int16_t),fd);

    fwrite(&timestamp,sizeof(char),sizeof(uint64_t),fd);
    fwrite(msg,sizeof(char),size+1,fd);

    /* Verify if padding must be added */
    int pad_num = (size+1)%8;
    char zero=0;
    if (pad_num != 0)
    {
        pad_num = 8 - pad_num;

        for (;pad_num > 0; --pad_num)
        {
            fwrite(&zero,sizeof(char),1,fd);
        }
    }


    //~ fprintf(fd,"%s",msg);
}


void CTFNewProcTimeEvent(FILE *fd, int16_t event_id, uint64_t timestamp,
//~ int cpu_num, double usage)
char * msg)
{
    int size = strlen(msg);
    /* Add event ID*/
    //~ fwrite(&event_id,sizeof(char),sizeof(int16_t),fd);

    fwrite(&timestamp,sizeof(char),sizeof(uint64_t),fd);
    fwrite(msg,sizeof(char),size+1,fd);

    /* Verify if padding must be added */
    int pad_num = (size+1)%8;
    char zero=0;
    if (pad_num != 0)
    {
        pad_num = 8 - pad_num;

        for (;pad_num > 0; --pad_num)
        {
            fwrite(&zero,sizeof(char),1,fd);
        }
    }
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

    fclose(FDMetadata);

#endif

    /* Create datastream file */

    fd = fopen("datastream", "w");

    CTFDataStreamGenerate(fd, UUID,sizeof(UUID),0,0);

    /* Add events */

    CTFNewCpuUsageEvent(fd, CPUUSAGE_EVENT_ID, 0xAD699E005810, "cpuusage 0 0.50");
    CTFNewCpuUsageEvent(fd, CPUUSAGE_EVENT_ID, 0xAD699E26F6C8, "proctime queue0 1000");
    CTFNewCpuUsageEvent(fd, CPUUSAGE_EVENT_ID, 0xAD699EC73638, "proctime queue2 2000");

    //~ CTFDataStreamPading(fd);

    fclose(fd);


    return 0;
}
