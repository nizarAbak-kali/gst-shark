#! /bin/bash

make -f MakeApp

sudo echo "TEST"
sudo nc -l 8080 > metadata_tcp &
sleep 1
GST_DEBUG=3 GST_SHARK_TRACE_LOC="file://./gst-shark-test;tcp://localhost:8080" ./gstsharkclient 

./gstsharkserver metadata_tcp


hexdump -C gst-shark-test/datastream > ref_datastream.txt
hexdump -C gst-shark-server/datastream > server_datastream.txt 
echo "Compare metadata"
diff -u gst-shark-test/metadata gst-shark-server/metadata
echo "Compare datastream"
diff -u ref_datastream.txt server_datastream.txt


#~ sudo echo "Files produced"
#~ ls -l metadata_tcp
#~ ls -l ./gst-shark-server/
#~ ls -l ./gst-shark-test

sudo echo "Test TCP"
babeltrace ./gst-shark-server
sudo echo "Test File"
babeltrace ./gst-shark-test
