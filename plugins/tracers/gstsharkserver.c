
#include <stdlib.h>
#include <sys/stat.h>
 #include <stdio.h>

#define DIR_NAME "./gst-shark-server/"

int main (int argc, char * argv[])
{
    FILE * fd;
    FILE * fd_metadata;
    FILE * fd_datastream;
    struct stat st;
    int size;
    int file_idx;
    char * file;
    int metadata_size;
    
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
    
    file = malloc(size);
    if (NULL ==file)
    {
        fprintf(stderr,"ERROR: memory allocation\n");
        return EXIT_SUCCESS;
    }
    
    fread(file,sizeof(char),size,fd);
    
    metadata_size = size;
    for (file_idx = 0; file_idx <= size; ++file_idx)
    {
        //~ printf("idx: %d %x\n",file_idx, (unsigned int)(0xFF &file[file_idx]));
        if ((unsigned int)0xC1 ==(unsigned int) (0xFF &file[file_idx]))
        {
            metadata_size = file_idx;
            //~ printf("HIT idx: %d\n",file_idx);
            break;
        }
    } 
    
    if (stat(DIR_NAME, &st) == -1)
    {
        mkdir(DIR_NAME, 0775);
    }
    
    fd_metadata = fopen(DIR_NAME"/metadata","w");
    fd_datastream = fopen(DIR_NAME"/datastream","w");
    
    if (NULL == fd_metadata || NULL == fd_datastream)
    {
        fprintf(stderr,"ERROR: metadata or datastream can not be opened\n");
    }
    
    fwrite(file,sizeof(char),metadata_size,fd_metadata);
    fwrite(&file[metadata_size],sizeof(char),size-metadata_size,fd_datastream);

    return EXIT_SUCCESS;
}
