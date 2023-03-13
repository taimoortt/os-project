#!/usr/bin/python3
TIMES=1
import matplotlib.pyplot as plt
import matplotlib
from collections import defaultdict
import numpy as np

INTRA = "pf"
FTYPE=".pdf"
COLORS=["dimgrey", "green", "cornflowerblue", "orange"]

# get the per-second cumulative bytes
def get_cumubytes(fname):
    end_ts = 10000 - 100
    ratio = 1 / 1000 # kbps -> Mbps
    cumu_bytes = defaultdict(lambda: 0)
    cumu_rbs = defaultdict(lambda: 0)
    with open(fname, "r") as fin:
        for line in fin:
            words = line.split(" ")
            if words[0] == "\t\tflow":
                slice_id = int(words[5])
                cumu_bytes[slice_id] += int(words[-1]) / end_ts * ratio
                cumu_rbs[slice_id] += int(words[7]) / end_ts
    print(cumu_bytes)
    # print(cumu_rbs)
    print("\n")
    return cumu_bytes, cumu_rbs

def get_throughput(dname):
    ratio = 8 / (1000 * 1000)
    for i in range(TIMES):
        get_cumubytes( dname + "1_" + str(i) + ".log")
    for i in range(TIMES):
        get_cumubytes( dname + "5_" + str(i) + ".log")

def plot_percell(fname):
    end_ts = 10000 - 100
    ratio = 1 / 1000 # kbps -> Mbps
    num_of_slice = 0
    num_of_cell = 0
    default_fontsize = 16
    with open(fname + ".log", "r") as fin:
        for line in fin:
            words = line.split(" ")
            if words[0] == "\t\tflow":
                cell_id = int(words[3])
                slice_id = int(words[5])
                if slice_id >= num_of_slice:
                    num_of_slice = slice_id + 1
                if cell_id >= num_of_cell:
                    num_of_cell = cell_id + 1
    rbs_per_cell = [[0 for i in range(num_of_cell)] for i in range(num_of_slice)]
    bws_per_cell = [[0 for i in range(num_of_cell)] for i in range(num_of_slice)]
    ues_per_cell = [[{} for i in range(num_of_cell)] for i in range(num_of_slice)]
    with open(fname + ".log", "r") as fin:
        for line in fin:
            words = line.split(" ")
            if words[0] == "\t\tflow":
                flow_id = int(words[1])
                cell_id = int(words[3])
                slice_id = int(words[5])
                rbs_per_cell[slice_id][cell_id] += int(words[7]) / end_ts
                bws_per_cell[slice_id][cell_id] += int(words[-1]) / end_ts * ratio
                ues_per_cell[slice_id][cell_id][flow_id] = 1
    x_array = np.arange(num_of_cell)
    fig, ax = plt.subplots(figsize=(8, 4))
    bar_width = 0.2
    ax.grid(axis="y", alpha=0.5)
    ax.bar( x_array - 1.5 * bar_width, rbs_per_cell[0], width=bar_width, label="Slice0", color=COLORS[0] )
    ax.bar( x_array - 0.5 * bar_width, rbs_per_cell[1], width=bar_width, label="Slice0", color=COLORS[1] )
    ax.bar( x_array + 0.5 * bar_width, rbs_per_cell[2], width=bar_width, label="Slice0", color=COLORS[2] )
    ax.bar( x_array + 1.5 * bar_width, rbs_per_cell[3], width=bar_width, label="Slice1", color=COLORS[3] )
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)
    ax.set_xlabel("Cell ID", fontsize=default_fontsize + 4)
    ax.set_ylabel("RBs-per-TTI", fontsize=default_fontsize + 4)
    ax.set_title(fname)
    # ax.legend()
    ax.tick_params(axis="both", labelsize=default_fontsize)
    plt.tight_layout()
    fig.savefig(fname + ".png")

    # for slices in ues_per_cell:
    #     for ues in slices:
    #         print(len(ues))


matplotlib.rcParams['pdf.fonttype'] = 42
matplotlib.rcParams['ps.fonttype'] = 42

# get_throughput("./logs/celledge_b")
plot_percell("./logs/celledge_b1_0")
plot_percell("./logs/celledge_b5_0")
