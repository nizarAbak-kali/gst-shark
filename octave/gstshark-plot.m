

GSTSHARK_SAVEFIG = 0;
TRUE = 1;

%~ figure_handlers

arg_list = argv ();
figs_num = 0;

for i = 1:nargin
  
    tracer_name = char(arg_list{i});

    switch tracer_name
        case 'cpuusage'
            disp('cpusage')
            plot_cpuusage
            figs_num = figs_num + 1;
        case 'framerate'
            disp('framerate')
            plot_framerate
            figs_num = figs_num + 1;
        case 'proctime'
            disp('proctime')
            plot_proctime
            figs_num = figs_num + 1;
        case 'interlatency'
            disp('interlatency')
            plot_interlatency
            figs_num = figs_num + 1;
        case 'scheduling'
            disp('scheduling')
            plot_scheduling
            figs_num = figs_num + 1;
        case '--savefig'
            GSTSHARK_SAVEFIG = 1;
        otherwise
            if (0 !=length(tracer_name))
                printf('octave: WARN: %s tracer does not exit',tracer_name)
            end
    end
end


if (TRUE == GSTSHARK_SAVEFIG)
    disp('Save figures...')
    for fig_idx = 1 : figs_num
        print tracer -dpdf -append
        close
    end
end
    
printf ("\n")

 
