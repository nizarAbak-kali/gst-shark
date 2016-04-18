#! /bin/bash

make -f MakeApp

sudo echo "TEST 1"
sudo nc -l 8080 &
sleep 1
GST_DEBUG=3 GST_SHARK_TRACE_LOC="file://./gst-shark0;tcp://127.0.0.1:8080" ./gstsharkclient 

echo "TEST 2"
sudo nc -l 1000 &
sleep 1
GST_SHARK_TRACE_LOC="file://./gst-shark1;tcp://127.0.0.1" ./gstsharkclient 

echo "TEST 3"
sudo nc -l 8080 &
sleep 1
GST_SHARK_TRACE_LOC="tcp://127.0.0.1:8080;file://./gst-shark2" ./gstsharkclient

echo "TEST 4"
sudo nc -l 1000 &
sleep 1
GST_SHARK_TRACE_LOC="tcp://127.0.0.1;file://./gst-shark3" ./gstsharkclient

echo "TEST 5"
sudo nc -l 1000 &
sleep 1
GST_SHARK_TRACE_LOC="tcp://localhost;file://./gst-shark4" ./gstsharkclient

echo "TEST 6"
# Test if there is not connection
GST_SHARK_TRACE_LOC="tcp://localhost;file://./gst-shark5" ./gstsharkclient

echo "TEST 7"
# Test if there is not connection
./gstsharkclient
