#!/usr/bin/env bash


DATE=`date "+%y-%m-%d_%H-%M-%S"`
export GST_SHARK_LOCATION="/tmp/cft_$DATE"
echo "$GST_SHARK_LOCATION"
unset GST_SHARK_CTF_DISABLE
echo "unset GST_SHARK_CTF_DISABLE"