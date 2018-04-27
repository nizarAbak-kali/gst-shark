#! /bin/bash

# Ridgerun GstShark


set -x
set -v

# Remove files
rm -f datastream.log
rm -f *.html
rm -f *.csv
for tracer in "${parser_group_list1[@]}"
do
    rm ${tracer}.log ${tracer}.mat -f
done

for tracer in "${parser_group_list2[@]}"
do
    rm ${tracer}.log
    rm ${tracer}_fields.log ${tracer}_fields.mat -f
    rm ${tracer}_values.log ${tracer}_values.mat -f
done


# Verify if there is at least a parameter
if [ $# -lt 1 ]
then
    WORKFOLDER=$GST_SHARK_LOCATION
    echo "taking default location $GST_SHARK_LOCATION"
fi

if [ ! -d $1 ] ;
then
    echo "Error: $1 is not a directory"
    echo "Try '$0 --help' for more information."
    exit
fi


processing_tracer_list=("proctime" "interlatency" "framerate" "scheduling" "cpuusage")
parser_group_list1=("proctime" "interlatency" "scheduling")
parser_group_list2=("cpuusage" "framerate")

#
rm -f tracer.pdf

# Create readable file
babeltrace $WORKFOLDER > datastream.log

# Loop through the tracer list 1
for tracer in "${parser_group_list1[@]}"
do
    echo "Loading ${tracer} events..."
    # Split the events in files
    grep -w ${tracer} datastream.log > ${tracer}.log
    # Get data columns
    awk '{print $1,$10,$13,$16}' ${tracer}.log > ${tracer}.mat
    # Create plots
done

set -x
# Loop through the tracer list 2
for tracer in "${parser_group_list2[@]}"
do
    echo "Loading ${tracer} events..."
    grep -w ${tracer} datastream.log > ${tracer}.log
    head -n 1 ${tracer}.log  > ${tracer}_fields.log
    # Count columns
    COL_RAW=$(awk '{ print NF }' ${tracer}_fields.log)
    COL_END=$(( COL_RAW - 3 ))
    # Create the awk parameter dynamically based in the amount of columns
    COUNTER=11
    AWK_PARAM_FILED_NAME='{print $8'
    AWK_PARAM_FIELD_VALUE='{print $1,$10'
    while [  $COUNTER -le $COL_END ]; do
        AWK_PARAM_FILED_NAME=${AWK_PARAM_FILED_NAME},'$'${COUNTER}
        AWK_PARAM_FIELD_VALUE=${AWK_PARAM_FIELD_VALUE},'$'$(( COUNTER + 2 ))
        let COUNTER=COUNTER+3
    done
    AWK_PARAM_FILED_NAME=${AWK_PARAM_FILED_NAME}'}'
    AWK_PARAM_FIELD_VALUE=${AWK_PARAM_FIELD_VALUE}'}'

    # Create a file with a list of field names
    awk "$AWK_PARAM_FILED_NAME" ${tracer}_fields.log > ${tracer}_fields.mat
    # Create a file with the timestamp and the list of field values for each event
    awk "$AWK_PARAM_FIELD_VALUE" ${tracer}.log > ${tracer}_values.mat
done
set +x
# Skip directory name
shift

# Parse options
while [[ $# -gt 0 ]]
do
    key="$1"
    case $key in
        -s|--savefig)
        SAVEFIG="--savefig"
        shift # past argument

        key="$1"
        case $key in
            png)
            FORMAT="png"
            shift
            ;;
            pdf)
            FORMAT="pdf"
            shift
            ;;
        esac
        ;;
        -l|--legend)
        shift # past argument

        key="$1"
        case $key in
            outside)
            LEGEND="northeastoutside"
            shift
            ;;
            inside)
            LEGEND="northeast"
            shift
            ;;
            extern)
            LEGEND="extern"
            shift
            ;;
        esac
        ;;
        -p|--persist)
        PERSIST="--persist"
        shift # past argument
        ;;
        *)
        echo "WARN: unkown \"$key\" option"
        shift
        ;;
    esac
done

# Create plots
octave -qf ${PERSIST} ./gstshark-plot.m "${processing_tracer_list[@]}" "${SAVEFIG}" "${FORMAT}" "${LEGEND}"

python3 gstshark-plot.py