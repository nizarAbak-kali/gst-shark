#! /bin/bash

hexdump -C kernel/channel0_0 > ref_datastream.txt
hexdump -C test/datastream > test_datastream.txt 
echo "Compare datastream"
diff -u ref_datastream.txt test_datastream.txt
