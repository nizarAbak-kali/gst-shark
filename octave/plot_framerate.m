#! /usr/bin/octave -qf

# Configuration
RESULT = 0;
FONTSIZE = 14;
LINEWIDTH = 1;

# Constants
TRUE = 1;
FALSE = 0;

[element_name_list, timestamp_mat, time_mat] = load_serie_timestamp_value('framerate.mat');

figure('Name','Frame rate')
plot(timestamp_mat',time_mat','linewidth',LINEWIDTH)
title('Frame rate','fontsize',FONTSIZE)
xlabel('time (seconds)','fontsize',FONTSIZE)
ylabel('Frame per second','fontsize',FONTSIZE)
legend(element_name_list)

%~ print tracer -dpdf -append

