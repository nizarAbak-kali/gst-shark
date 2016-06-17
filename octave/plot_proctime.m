#! /usr/bin/octave -qf

# Configuration
RESULT = 1;
FONTSIZE = 14;
LINEWIDTH = 1;

# Constants
TRUE = 1;
FALSE = 0;

# Open tracer data
fileID = fopen('proctime.mat');

count = 1;
# Compute How many series need to be created

[timestamp count] = fscanf(fileID,'[%s]');
%~ [from_pad count] = fscanf(fileID,'%s\"');
[element count] = fscanf(fileID,'%s\"');
[time count] = fscanf(fileID,'%d');

pad_name_list = {element};
pad_freq_list = 1;

while (count == 1)
    [timestamp count] = fscanf(fileID,'[%s]');
    if (count == 0)
        break
    end
    %~ [from_pad count] = fscanf(fileID,'%s\"');
    [element count] = fscanf(fileID,'%s\"');
    [time count] = fscanf(fileID,'%d');

    pad_name_list_len = length(pad_name_list);
    for list_idx = 1:pad_name_list_len
        if (1 == strcmp(char(pad_name_list{list_idx}),element))
            pad_freq_list(list_idx) = pad_freq_list(list_idx) + 1;
            pad_found = TRUE;
        end
    end
    if (pad_found == FALSE)
        pad_name_list_len = length(pad_name_list) + 1;
        pad_name_list{pad_name_list_len} = element;
        pad_freq_list(pad_name_list_len) = 1;
    end
    pad_found = FALSE;

    %~ printf('%s %s %s %d\n',timestamp,from_pad,element,time);
end

if (RESULT)
    for list_idx = 1:length(pad_name_list)
        printf('%s %d \n',pad_name_list{list_idx},pad_freq_list(list_idx));
    end
end

# Creata matrix to store the data
timestamp_mat = nan(length(pad_name_list),max(pad_freq_list));
time_mat = nan(size(timestamp_mat));

# Move to the beginning of a file
frewind(fileID)
data_mat_idx_list = ones(1,length(pad_name_list));
pad_name_list_len = length(pad_name_list);
count = 1;
while (count == 1)
    [timestamp count] = fscanf(fileID,'[%s]');
    if (count == 0)
        break
    end
    %~ [from_pad count] = fscanf(fileID,'%s\"');
    [element count] = fscanf(fileID,'%s\"');
    [time count] = fscanf(fileID,'%d');
    # Match the event with a pad name
    for list_idx = 1:pad_name_list_len
        if (1 == strcmp(char(pad_name_list{list_idx}),element))
            [timestamp_array] = sscanf(timestamp,'%d:%d:%f]');
            timestamp_val = timestamp_array(3) + (timestamp_array(2) * 60) + timestamp_array(1);
            timestamp_mat(list_idx,data_mat_idx_list(list_idx)) = timestamp_val;
            time_mat(list_idx,data_mat_idx_list(list_idx)) = time;
            data_mat_idx_list(list_idx) = data_mat_idx_list(list_idx) + 1;
        end
    end
end

figure('Name','Processing time')
plot(timestamp_mat',time_mat','linewidth',LINEWIDTH)
title('Processing time','fontsize',FONTSIZE)
xlabel('time (seconds)','fontsize',FONTSIZE)
ylabel('time (nanoseconds)','fontsize',FONTSIZE)
legend(pad_name_list)

print tracer -dpdf -append


fclose('all');