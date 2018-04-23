#! /usr/bin/python3
import sys
import logging as log
import cpuusage_process
from   tracer import Tracer

if __name__ == '__main__':
    arg_list = sys.argv
    tracer = Tracer(str(arg_list))

    for option in arg_list :

        if option in "cpuusage":
            log.info("Processing cpusage...")
            (cpu_name_list, timestamp_mat, cpu_mat) = cpuusage_process.cpuusage_process()
            if (timestamp_mat[0,0] != -1):
                tracer.

        elif option in "cpuusage":