#! /bin/bash 


source gstshark-init.sh

GST_DEBUG="GST_TRACER:7" GST_TRACERS="cpuusage;proctime;interlatency;scheduletime;graphic;queue;buffer"\
     gst-launch-1.0 videotestsrc num-buffers=500 ! videorate max-rate=15 ! fakesink

./gstshark-plot.sh
