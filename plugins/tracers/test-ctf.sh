#! /bin/bash

echo "Compile concept api application" 
gcc -o ctf_app ctf-api-test.c

echo "Create matadata and datastream"
./ctf_app;

echo "Compare ref datastream and test datastream" 
cp datastream metadata test
#~ ./compare.sh

echo "Read the CTF file generated with babeltrace" 
babeltrace ./test
