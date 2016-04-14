#! /bin/bash

make -f MakeApp

GST_SHARK_TRACE_LOC="file:///tmp/gst-shark;tcp://127.0.0.1:8080" ./gstsharkclient 
