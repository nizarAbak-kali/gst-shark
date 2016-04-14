#! /bin/bash

make -f MakeApp

echo "TEST 1"
GST_SHARK_TRACE_LOC="file:///tmp/gst-shark;tcp://127.0.0.1:8080" ./gstsharkclient 
echo "TEST 2"
GST_SHARK_TRACE_LOC="file:///tmp/gst-shark;tcp://127.0.0.1" ./gstsharkclient 
echo "TEST 3"
GST_SHARK_TRACE_LOC="tcp://127.0.0.1:8080;file:///tmp/gst-shark" ./gstsharkclient

