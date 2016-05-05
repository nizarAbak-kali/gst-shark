#! /bin/bash

#~ make -f MakeApp

mkdir gst-shark-server

export GST_DEBUG="GST_TRACER:7,2"

sudo echo "Framerate"
sudo nc -l 8080 > metadata_tcp &
sleep 1
GST_SHARK_TRACE_LOC="file://./gst-shark-test;tcp://localhost:8080" GST_TRACER_PLUGINS="framerate" GST_PLUGIN_PATH_1_0=/usr/lib/x86_64-linux-gnu/gstreamer-1.0/ /home/mleiva/devdirs/gst-1.7.1/gstreamer-1.7.1/build/bin/gst-launch-1.0 videotestsrc num-buffers=50 ! videorate max-rate=15 ! fakesink sync=true
./gstsharkserver metadata_tcp
babeltrace ./gst-shark-server

echo "Proctime"

sudo nc -l 8080 > metadata_tcp &
sleep 1
GST_SHARK_TRACE_LOC="file://./gst-shark-test;tcp://localhost:8080" GST_TRACER_PLUGINS="proctime" GST_PLUGIN_PATH_1_0=/usr/lib/x86_64-linux-gnu/gstreamer-1.0/ /home/mleiva/devdirs/gst-1.7.1/gstreamer-1.7.1/build/bin/gst-launch-1.0 videotestsrc num-buffers=5 ! identity sleep-time=8500 ! \
tee name=t t. ! queue ! identity sleep-time=50000 ! fakesink t. ! queue ! identity sleep-time=30000 ! fakesink
./gstsharkserver metadata_tcp
babeltrace ./gst-shark-server

echo "cpuusage"

sudo nc -l 8080 > metadata_tcp &
sleep 1
GST_SHARK_TRACE_LOC="file://./gst-shark-test;tcp://localhost:8080" GST_TRACER_PLUGINS="cpuusage" GST_PLUGIN_PATH_1_0=/usr/lib/x86_64-linux-gnu/gstreamer-1.0/ /home/mleiva/devdirs/gst-1.7.1/gstreamer-1.7.1/build/bin/gst-launch-1.0 videotestsrc num-buffers=50 ! queue  ! fakesink
./gstsharkserver metadata_tcp
babeltrace ./gst-shark-server

echo "scheduletime"

sudo nc -l 8080 > metadata_tcp &
sleep 1
GST_SHARK_TRACE_LOC="file://./gst-shark-test;tcp://localhost:8080" GST_TRACER_PLUGINS="scheduletime" GST_PLUGIN_PATH_1_0=/usr/lib/x86_64-linux-gnu/gstreamer-1.0/ /home/mleiva/devdirs/gst-1.7.1/gstreamer-1.7.1/build/bin/gst-launch-1.0 videotestsrc num-buffers=5 ! queue  ! fakesink
./gstsharkserver metadata_tcp
babeltrace ./gst-shark-server

echo "interlatency"

sudo nc -l 8080 > metadata_tcp &
sleep 1
GST_SHARK_TRACE_LOC="file://./gst-shark-test;tcp://localhost:8080" GST_TRACER_PLUGINS="interlatency" GST_PLUGIN_PATH_1_0=/usr/lib/x86_64-linux-gnu/gstreamer-1.0/ /home/mleiva/devdirs/gst-1.7.1/gstreamer-1.7.1/build/bin/gst-launch-1.0 videotestsrc num-buffers=5 ! queue  ! fakesink
./gstsharkserver metadata_tcp
babeltrace ./gst-shark-server

echo "All"

sudo nc -l 8080 > metadata_tcp &
sleep 1
GST_SHARK_TRACE_LOC="file://./gst-shark-test;tcp://localhost:8080" GST_TRACER_PLUGINS="cpuusage;interlatency;scheduletime;framerate;proctime" GST_PLUGIN_PATH_1_0=/usr/lib/x86_64-linux-gnu/gstreamer-1.0/ /home/mleiva/devdirs/gst-1.7.1/gstreamer-1.7.1/build/bin/gst-launch-1.0 videotestsrc num-buffers=50 ! queue  ! fakesink
./gstsharkserver metadata_tcp
babeltrace ./gst-shark-server
