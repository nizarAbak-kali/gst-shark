#! /bin/bash

echo "parametros $#"

if [ $# -lt 1 ]
then
    echo "Error: A directory name must be given"
fi

if [ ! -d $1 ]
then
    echo "Error: $1 is not a directory"
fi

# Create readable file
babeltrace $1 > datastream.log
# Split the events in files
awk '{print $1,$10,$13,$16}' interlatency.log > interlatency.mat
# Create plots
./plot_interlatency.m
