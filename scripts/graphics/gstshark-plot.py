#! /usr/bin/python3
import os
import csv
import re
import sys
import scipy
import logging as log
import plotly.plotly as py
import plotly.graph_objs as go
import plotly.figure_factory as FF
import plotly.offline as pyoff
import numpy as np
import pandas as pd



def plot_cpu(cpuusage_file):

    df = pd.read_csv(cpuusage_file)
    fields = df.keys()

    trace_list = []
    layout_list = []

    for field in fields[1:]:
        trace = go.Scatter(x=df['time'], y=df[field],
                           mode='lines', name=field)
        layout = go.Layout(title='Plot full info',
                       plot_bgcolor='rgb(230, 230,230)', connectgaps='true',
                           xaxis=dict(title="time"), yaxis=dict(title="data"))
        trace_list.append(trace)
        layout_list.append(layout)

    return trace_list, layout_list


def plot_framerate(framerate_file):

    df = pd.read_csv(framerate_file)
    fields = df.keys()

    trace_list = []
    layout_list = []

    for field in fields[1:]:
        trace = go.Scatter(x=df['time'], y=df[field], connectgaps='true',
                           mode='lines', name=field)
        layout = go.Layout(title='Plot framerate info',
                           plot_bgcolor='rgb(230, 230,230)',
                           xaxis=dict(title="time"), yaxis=dict(title="data"))
        trace_list.append(trace)
        layout_list.append(layout)

    return trace_list, layout_list


def plot_interlatency(interlatency_file):
    df = pd.read_csv(interlatency_file)
    fields = df.keys()

    trace_list = []
    layout_list = []

    for field in fields[1:]:
        trace = go.Scatter(x=df['time'], y=df[field], connectgaps='true',
                           mode='lines', name=field)
        layout = go.Layout(title='Plot interlatency info',
                           plot_bgcolor='rgb(230, 230,230)',
                           xaxis=dict(title="time"), yaxis=dict(title="latency"))
        trace_list.append(trace)
        layout_list.append(layout)

    return trace_list, layout_list


def plot_scheduling(scheduling_file):
    df = pd.read_csv(scheduling_file)
    fields = df.keys()

    trace_list = []
    layout_list = []

    for field in fields[1:]:
        trace = go.Scatter(x=df['time'], y=df[field], connectgaps=True,
                           mode='lines', name=field)
        layout = go.Layout(title='Plot scheduling info',
                           plot_bgcolor='rgb(230, 230,230)',
                           xaxis=dict(title="time"), yaxis=dict(title="elapsed_time"))
        trace_list.append(trace)
        layout_list.append(layout)

    return trace_list, layout_list

def plot_proctime(proctime_file):
    df = pd.read_csv(proctime_file)
    fields = df.keys()

    trace_list = []
    layout_list = []

    for field in fields[1:]:
        trace = go.Scatter(x=df['time'], y=df[field],
                           mode='lines', name=field)
        layout = go.Layout(title='Plot process time info',
                           plot_bgcolor='rgb(230, 230,230)',
                           xaxis=dict(title="time"), yaxis=dict(title="process time"))
        trace_list.append(trace)
        layout_list.append(layout)

    return trace_list, layout_list


def get_mat_row(mat_file, index=0):
    row = []
    with open(mat_file, "r") as file :
        for line in file.readlines():
            splited_line = re.split(" ", line)
            row.append(splited_line[index])
    return row


def csv_builder_interlatency(values_filename):
    tracer_name = "interlatency"
    tracer_fields = list()
    tracer_fields.append("time")
    row_sink = get_mat_row(values_filename, 2)
    row_sink = set(row_sink)
    for i in row_sink:
        tracer_fields.append(i.replace(",",""))
    tracer_fields.append("latency")

    csv_file_name = tracer_name + ".csv"

    with open(csv_file_name, 'w', newline='') as csv_file:
        tracerwriter = csv.DictWriter(csv_file,fieldnames=tracer_fields)

        tracerwriter.writeheader()
        content = []
        with open(values_filename, 'r') as value_file:
            for line in value_file:
                content.append(line.strip())
        for line in content:
            line = line.replace(',', '')
            values = re.split(" ", line)
            tracerwriter.writerow({'time': values[0], values[2]: values[3]})

    return tracer_name, tracer_fields


def csv_builder_scheduling(values_filename):
    tracer_name = "scheduling"
    tracer_fields = list()
    tracer_fields.append("time")
    row_sink = get_mat_row(values_filename, 1)
    row_sink = set(row_sink)
    for i in row_sink:
        tracer_fields.append(i.replace(',', ''))
    tracer_fields.append("elapsed_time")

    csv_file_name = tracer_name + ".csv"

    with open(csv_file_name, 'w', newline='') as csv_file:
        tracerwriter = csv.DictWriter(csv_file, fieldnames=tracer_fields)
        tracerwriter.writeheader()

        content = []
        with open(values_filename, 'r') as value_file:
            for line in value_file:
                content.append(line.strip())
        for line in content:
            line = line.replace(',', '')
            values = re.split(" ", line)
            tracerwriter.writerow({'time': values[0], values[1]: values[2]})

    return tracer_name, tracer_fields


def csv_builder_proctime(values_filename):
    tracer_name = "proctime"
    tracer_fields = list()
    tracer_fields.append("time")
    row_sink = get_mat_row(values_filename, 1)
    row_sink = set(row_sink)
    for i in row_sink:
        tracer_fields.append(i.replace(',', ''))
    tracer_fields.append("cpu_time")

    csv_file_name = tracer_name + ".csv"

    with open(csv_file_name, 'w', newline='') as csv_file:

        tracerwriter = csv.DictWriter(csv_file, fieldnames=tracer_fields)
        tracerwriter.writeheader()

        content = []
        with open(values_filename, 'r') as value_file:
            for line in value_file:
                content.append(line.strip())
        for line in content:
            line = line.replace(',', '')
            values = re.split(" ", line)
            tracerwriter.writerow({'time': values[0], values[1]: values[2]})

    return tracer_name, tracer_fields


def csv_builder():
    tuple_list = []
    normal_list = []

    for file_name in os.listdir("."):
        if file_name.endswith("fields.mat"):
            mat_field_file_name = file_name
            tracer_name = re.split("_", mat_field_file_name)[0]
            mat_value_file_name = tracer_name+"_values.mat"
            tuple_list.append((mat_field_file_name, mat_value_file_name))
        elif file_name.endswith(".mat") and not file_name.endswith("values.mat"):
            normal_list.append(file_name)

    csv_file_to_ret = []

    for values_filename in normal_list:
        tracer_fields = []
        tracer_name = "None"
        if values_filename.split(".")[0] == "interlatency":
            t_n, t_f = csv_builder_interlatency(values_filename)
            tracer_name = t_n
            tracer_fields.extend(t_f)

        if values_filename.split(".")[0] == "scheduling":
            t_n, t_f = csv_builder_scheduling(values_filename)
            tracer_name = t_n
            tracer_fields.extend(t_f)

        if values_filename.split(".")[0] == "proctime":
            t_n, t_f = csv_builder_proctime(values_filename)
            tracer_name = t_n
            tracer_fields.extend(t_f)

        csv_file_name = tracer_name + ".csv"

        csv_file_to_ret.append(csv_file_name)

    for fields_file_name, values_filename in tuple_list:

        tracer_fields = []

        with open(fields_file_name, 'r') as file:
            tracer_fields = re.split(" ", file.read())
        tracer_fields = [i.strip() for i in tracer_fields]
        tracer_fields.insert(0, "time")

        csv_file_name = fields_file_name.split("_")[0]+".csv"
        with open(csv_file_name, 'w', newline='') as csv_file:
            tracerwriter = csv.writer(csv_file, delimiter=',',
                                      quoting=csv.QUOTE_NONNUMERIC)
            tracerwriter.writerow(tracer_fields)

            content = []
            with open(values_filename, 'r') as value_file:
                for line in value_file:
                    content.append(line.strip())
            for line in content:
                values = re.split(" ", line)
                tracerwriter.writerow(values)

        csv_file_to_ret.append(csv_file_name)
    print(csv_file_to_ret)
    return csv_file_to_ret


def plot(csv_files):

    csv_files_set = list(set(csv_files))
    for csv_file in csv_files_set:
        df = pd.read_csv(csv_file)
        fields = df.keys()

        fig = None
        trace_list = []
        layout_list = []

        for field in fields[1:]:
            trace = go.Scatter(x=df['time'], y=df[field],
                               mode='lines', name=field)
            layout = go.Layout(title='Plot full info',
                           plot_bgcolor='rgb(230, 230,230)',
                               xaxis=dict(title="time"), yaxis=dict(title="data"))
            trace_list.append(trace)
            layout_list.append(layout)

        fig = go.Figure(data=trace_list , layout=layout_list[0])
        pyoff.plot(figure_or_data=fig, link_text="full.analyse", filename="full.html")


if __name__ == '__main__':

        csv_files = csv_builder()
        traces, layouts = plot_proctime(csv_files[2])
        #plot_cpu()

        fig = go.Figure(data=traces, layout=layouts[0])
        pyoff.plot(figure_or_data=fig, link_text="full.analyse", filename="full.html")

        #plot(csv_files)