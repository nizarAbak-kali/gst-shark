/* gcc -o ctf_app ctf-api-test.c */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define BYTE_ORDER_LE (1)

typedef enum {
    CPUUSAGE_EVENT_ID,
} event_id;

int Magic = 0xC1FC1FC1;
char UUID[] = {0xB2,0x96,0x90,0x8F,0xBF,0x96,0x42,0x33,0x94,0xD9,0xD8,0x55,0x4D,0x16,0xBC,0x7A};
char * UUIDstring = "b296908f-bf96-4233-94d9-d8554d16bc7a";

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
"		uint64_t content_size;\n"
"		uint64_t packet_size;\n"
"	};\n"
"	typealias integer { size = 64; align = 64; signed = false; } := uint64_t;\n"
"	event.header := struct {\n"
"		enum : uint16_t { compact = 0 ... 65534, extended = 65535 } id;\n"
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

static const char metadata_stream_event_header_timestamp[] =
"	typealias integer { size = 64; align = 64; signed = false; } := uint64_t;\n"
"	event.header := struct {\n"
"		uint64_t timestamp;\n"
"	};\n";


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
    content_size = 0x88090000;
    fwrite(&content_size,sizeof(char),sizeof(uint64_t),fd);
    /* Packet size */
    packet_size = 262144;
    fwrite(&packet_size,sizeof(char),sizeof(uint64_t),fd);
    /* ? ? ? ? */
    uint32_t unknown = 0;
    fwrite(&unknown,sizeof(char),sizeof(uint32_t),fd);
}

void CTFMetadataGenerate(FILE *fd, int major, int minor, char * UUID, int byte_order)
{
    /* Convert UUID */
    //~ bt_uuid_unparse(UUID, UUIDstring);
    
    fprintf(fd, metadata_fmt,
    major, /* major */
    minor, /* minor */
    UUIDstring,
    byte_order ? "be" : "le"
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
    fwrite(&event_id,sizeof(char),sizeof(int16_t),fd);
    
    fwrite(&timestamp,sizeof(char),sizeof(uint64_t),fd);
    fwrite(msg,sizeof(char),size+1,fd);
    
    //~ fprintf(fd,"%s",msg);
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
    
    fd = fopen("file.ctf", "w");
    
    CTFDataStreamGenerate(fd, UUID,sizeof(UUID),0,0);
    
    /* Add events */
    
    CTFNewCpuUsageEvent(fd, CPUUSAGE_EVENT_ID, 0xAA, "Event1-message"); 
    CTFNewCpuUsageEvent(fd, CPUUSAGE_EVENT_ID, 0xBB, "Event2-message"); 
     
    fclose(fd);
    
    
    return 0;
}
