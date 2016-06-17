#! /usr/bin/octave -qf

# Configuration
RESULT = 1;
FONTSIZE = 14;
LINEWIDTH = 1;

# Constants
TRUE = 1;
FALSE = 0;

# Open tracer data
fileID = fopen('cpuusage.mat');


[timestamp count] = fscanf(fileID,'[%s]');

# Compute how many CPUs has each event
cpu_num = 0;
while (1 == count)
    [cpu_val, count] = fscanf(fileID,'%f,"');
    cpu_num = cpu_num + 1;
end
cpu_num = cpu_num - 1;
printf('Cpu num %d\n',cpu_num)

# Move to the beginning of a file
frewind(fileID)
# Compute how many evens has the log
event_count = 0;
count = 1;
while (1 == count)
    [char_val, count] = fread(fileID,1,'char');
    if (char_val == '[')
        event_count = event_count + 1;
    end
end

printf('Event num %d\n',event_count)

frewind(fileID)

cpu_idx = 1;
count = 1
while (count == 1)
    [timestamp count] = fscanf(fileID,'[%s]');
    timestamp
    if (count == 0)
        break
    end
    
    for cpu_idx = 1 : (cpu_num - 1)
        [cpu_val, count] = fscanf(fileID,'%f,"');
        cpu_val
    end
    [cpu_val, count] = fscanf(fileID,'%f\n"');
    cpu_val
    
    # Creata matrix to store the data
    %~ timestamp_mat = nan(length(serie_name_list),max(serie_freq_list));
    %~ value_mat = nan(size(timestamp_mat));
end
    
fclose(fileID);

%~ figure('Name','Frame rate')
%~ plot(timestamp_mat',time_mat','linewidth',LINEWIDTH)
%~ title('Frame rate','fontsize',FONTSIZE)
%~ xlabel('time (seconds)','fontsize',FONTSIZE)
%~ ylabel('Frame per second','fontsize',FONTSIZE)
%~ legend(element_name_list)
%~ 
%~ print tracer -dpdf -append


