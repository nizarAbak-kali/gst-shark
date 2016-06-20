

arg_list = argv ();

for i = 1:nargin
  
    tracer_name = char(arg_list{i});

    if ('-' == tracer_name(1))
    continue
    end

    switch tracer_name
        case 'cpuusage'
            disp('cpusage')
            plot_cpuusage
        case 'framerate'
            disp('framerate')
            plot_framerate
        case 'proctime'
            disp('proctime')
            plot_proctime
        case 'interlatency'
            disp('interlatency')
            plot_interlatency
        case 'scheduling'
            disp('scheduling')
            plot_scheduling
        otherwise
            printf('WARN: %s tracer does not exit',tracer_name )
    end
end
  
    
printf ("\n")

 
