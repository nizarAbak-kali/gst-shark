#! /bin/bash

make -f MakeApp

echo "TEST 1"
GST_SHARK_TRACE_LOC="file://./gst-shark1;tcp://127.0.0.1:8080" ./gstparser 

echo "TEST 2"
GST_SHARK_TRACE_LOC="file://./gst-shark2;tcp://127.0.0.1" ./gstparser 

echo "TEST 3"
GST_SHARK_TRACE_LOC="tcp://127.0.0.1:8080;file://./gst-shark3" ./gstparser

echo "TEST 4"
GST_SHARK_TRACE_LOC="tcp://127.0.0.1;file://./gst-shark4" ./gstparser

echo "TEST 5"
GST_SHARK_TRACE_LOC="tcp://localhost;file://./gst-shark5" ./gstparser

echo "TEST 6"
# Test if there is not connection
GST_SHARK_TRACE_LOC="tcp://localhost;file://./gst-shark6" ./gstparser

echo "TEST 7"
# Test if there is not destination directory
./gstparser
echo "TEST 8"
GST_SHARK_TRACE_LOC="tcp://localhost;./gst-shark8" ./gstparser

echo "TEST 9"
GST_SHARK_TRACE_LOC="./gst-shark9;tcp://localhost" ./gstparser


