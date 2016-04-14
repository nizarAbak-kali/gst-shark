#! /bin/bash

make -f MakeApp

sudo echo "TEST 1"
sudo nc -l 8080 &
sleep 1
GST_SHARK_TRACE_LOC="file:///tmp/gst-shark;tcp://127.0.0.1:8080" ./gstsharkclient 

echo "TEST 2"
sudo nc -l 1000 &
sleep 1
GST_SHARK_TRACE_LOC="file:///tmp/gst-shark;tcp://127.0.0.1" ./gstsharkclient 

echo "TEST 3"
sudo nc -l 8080 &
sleep 1
GST_SHARK_TRACE_LOC="tcp://127.0.0.1:8080;file:///tmp/gst-shark" ./gstsharkclient

echo "TEST 4"
sudo nc -l 1000 &
sleep 1
GST_SHARK_TRACE_LOC="tcp://127.0.0.1;file:///tmp/gst-shark" ./gstsharkclient

echo "TEST 5"
sudo nc -l 1000 &
sleep 1
GST_SHARK_TRACE_LOC="tcp://localhost;file:///tmp/gst-shark" ./gstsharkclient
