#! /bin/bash

make -f MakeApp

sudo echo "TEST 1"
sudo nc -l 8080 > metadata_tcp &
sleep 1
GST_DEBUG=3 GST_SHARK_TRACE_LOC="file://./gst-shark-test;tcp://localhost:8080" ./gstsharkclient 

#~ ./gstsharkserver metadata_tcp
#~ 
#~ sudo echo "Files produced"
#~ ls -l metadata_tcp
#~ ls -l ./gst-shark-server/
#~ ls -l ./gst-shark-test
#~ 
#~ sudo echo "Test TCP"
#~ babeltrace ./gst-shark-server
#~ sudo echo "Test File"
#~ babeltrace ./gst-shark-test
