#! /bin/bash

# Verify if there is at least a parameter
if [ $# -lt 1 ]
then
    echo "Error: A directory name must be given"
fi

if [ ! -d $1 ]
then
    echo "Error: $1 is not a directory"
fi

tracer_list=("proctime" "interlatency" "framerate" "scheduling")

#
rm -f tracer.pdf

# Loop through the tracer list
for tracer in "${tracer_list[@]}"
do
    echo "Tracer ${tracer}"
   # Create readable file
    babeltrace $1 > datastream.log
    # Split the events in files
    grep -w ${tracer} datastream.log > ${tracer}.log
    # Get data columns
    awk '{print $1,$10,$13,$16}' ${tracer}.log > ${tracer}.mat
    # Create plots
    ./plot_${tracer}.m
done





