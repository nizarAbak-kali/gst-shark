#! /bin/bash

echo "Compile concept api application"
gcc -Wall -g ctf-api-test.c -o ctf_app  -pthread -I/usr/include/gstreamer-1.0 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include  -L/usr/lib/x86_64-linux-gnu -lgstreamer-1.0 -lgobject-2.0 -lglib-2.0
#~ gcc -o ctf_app ctf-api-test.c

echo "Create matadata and datastream"
./ctf_app;

#~ echo "Compare ref datastream and test datastream" 
cp datastream metadata test
#~ ./compare.sh

echo "Read the CTF file generated with babeltrace" 
babeltrace ./test
