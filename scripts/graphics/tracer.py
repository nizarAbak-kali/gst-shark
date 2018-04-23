import numpy as np

class Tracer:
    def __init__(self, args):
        self.arg_list = args
        self.cpuusage = TracerData()
        self.framerate = TracerData()
        self.proctime = TracerData()
        self.interlatency = TracerData()
        self.scheduling = TracerData()


class TracerData:
    def __init__(self):
        self.name_list = []
        self.timestamp_map = np.mat()
        self.values_map = np.mat()