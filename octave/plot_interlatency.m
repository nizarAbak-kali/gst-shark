#! /usr/bin/octave -qf

# Configuration
RESULT = 1;

# Constants
TRUE = 1;
FALSE = 0;

# Open tracer data
fileID = fopen('interlatency.mat');

count = 1;
# Compute How many series need to be created

[timestamp count] = fscanf(fileID,'[%s]');
[from_pad count] = fscanf(fileID,'%s\"');
[to_pad count] = fscanf(fileID,'%s\"');
[time count] = fscanf(fileID,'%d');
printf('%s %s %s %d\n',timestamp,from_pad,to_pad,time);

# Store the source element
src_pad = from_pad;
pad_name_list = {to_pad};
pad_freq_list = 1;

while (count == 1) 
    [timestamp count] = fscanf(fileID,'[%s]');
    if (count == 0)
        break
    end
    [from_pad count] = fscanf(fileID,'%s\"');
    [to_pad count] = fscanf(fileID,'%s\"');
    [time count] = fscanf(fileID,'%d');
    
    pad_name_list_len = length(pad_name_list);
    for list_idx = 1:pad_name_list_len
        if (1 == strcmp(char(pad_name_list{list_idx}),to_pad))
            pad_freq_list(list_idx) = pad_freq_list(list_idx) + 1;
            pad_found = TRUE;
        end
    end
    %~ pad_found
    if (pad_found == FALSE)
        pad_name_list_len = length(pad_name_list) + 1;
        pad_name_list{pad_name_list_len} = to_pad;
        pad_freq_list(pad_name_list_len) = 1;
    end
    pad_found = FALSE;
    
    printf('%s %s %s %d\n',timestamp,from_pad,to_pad,time);
end

if (RESULT)
    for list_idx = 1:length(pad_name_list)
        printf('%s %d \n',pad_name_list{list_idx},pad_freq_list(list_idx));
    end
end

fclose('all');
