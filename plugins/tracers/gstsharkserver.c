
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>
 #include <stdio.h>

#define DIR_NAME "./gst-shark-server/"

#define TCP_METADATA_ID  (0x01)
#define TCP_DATASTREAM_ID  (0x02)

typedef uint8_t  tcp_header_id;
typedef uint32_t tcp_header_length;

int main (int argc, char * argv[])
{
    FILE * fd;
    FILE * fd_metadata;
    FILE * fd_datastream;
    //~ struct stat st;
    int size;
    //~ int file_idx;
    char * file;
    //~ int metadata_size;
    tcp_header_id header_id;
    int header_id_position;
    tcp_header_length header_length;
    int file_read;
    
    if (argc < 2)
    {
        fprintf(stderr,"ERROR: File name not provided\n");
    }
    
    fd = fopen(argv[1],"r");
    
    if (NULL == fd)
    {
        fprintf(stderr,"ERROR: File name can not be opened\n");
    }
    
    fseek(fd, 0L, SEEK_END);
    size = ftell(fd);
    //~ printf("size: %d\n",size);
    fseek(fd, 0L, SEEK_SET);
    //stat(argv[1], &st);
    //size = st.st_size;
    printf("%d: file size %d\n",__LINE__,size);
    
    file = malloc(size);
    if (NULL ==file)
    {
        fprintf(stderr,"ERROR: memory allocation\n");
        return EXIT_SUCCESS;
    }
    
    fread(file,sizeof(char),size,fd);
    
    fd_metadata = fopen(DIR_NAME"/metadata","w");
    fd_datastream = fopen(DIR_NAME"/datastream","w");
    
    if (NULL == fd_metadata || NULL == fd_datastream)
    {
        fprintf(stderr,"ERROR: metadata or datastream can not be opened\n");
    }
    
    file_read = 0;
    while (file_read <= size)
    {
        header_id = file[file_read];
        printf("%d: file_read %d\n",__LINE__,file_read);
        header_id_position = file_read;
        printf("%d: file_read %d\n",__LINE__,file_read);
        file_read += sizeof(header_id);
        printf("%d: file_read %d\n",__LINE__,file_read);
        header_length = *((tcp_header_length*)(&file[file_read]));
        printf("%d: file_read %d\n",__LINE__,file_read);
        file_read += sizeof(tcp_header_length) ;
                printf("%d: file_read %d\n",__LINE__,file_read);
        printf("header_id: 0x%X header_length %d\n",header_id, header_length);
        if (header_length < 0)
        {
            break;
        }
        switch (header_id)
        {
            case TCP_METADATA_ID:
                fwrite(&file[file_read],sizeof(char),header_length,fd_metadata);
                break;
            case TCP_DATASTREAM_ID:
                fwrite(&file[file_read],sizeof(char),header_length,fd_datastream);
                break;
            default:
                fprintf(stderr,"ERROR: unknown TCP ID header [0x%X] at position %d\n",header_id,header_id_position);
                return EXIT_SUCCESS;
        }
        printf("%d: file_read %d\n",__LINE__,file_read);
        file_read += header_length;
        printf("%d: file_read %d\n",__LINE__,file_read);
    }
    
    fclose(fd_metadata);
    fclose(fd_datastream);
    //~ for (file_idx = 0; file_idx <= size; ++file_idx)
    //~ {
        //~ 
        //~ printf("idx: %d %x\n",file_idx, (unsigned int)(0xFF &file[file_idx]));
        //~ if ((unsigned int)0xC1 ==(unsigned int) (0xFF &file[file_idx]))
        //~ {
            //~ metadata_size = file_idx;
            //~ printf("HIT idx: %d\n",file_idx);
            //~ break;
        //~ }
    //~ } 
    //~ 
    //~ if (stat(DIR_NAME, &st) == -1)
    //~ {
        //~ mkdir(DIR_NAME, 0775);
    //~ }
    //~ 
    //~ fd_metadata = fopen(DIR_NAME"/metadata","w");
    //~ fd_datastream = fopen(DIR_NAME"/datastream","w");
    //~ 
    //~ if (NULL == fd_metadata || NULL == fd_datastream)
    //~ {
        //~ fprintf(stderr,"ERROR: metadata or datastream can not be opened\n");
    //~ }
    //~ 
    //~ fwrite(file,sizeof(char),metadata_size,fd_metadata);
    //~ fwrite(&file[metadata_size],sizeof(char),size-metadata_size,fd_datastream);
//~ 
    return EXIT_SUCCESS;
}
