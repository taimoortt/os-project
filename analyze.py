#!/usr/bin/python3
TIMES=2
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
    return cumu_bytes, cumu_rbs

def get_throughput(dname):
    aggre_bws = []
    for i in range(TIMES):
        cumu_bytes, _ = get_cumubytes( dname + str(i) + ".log")
        aggre_bw = 0
        for k, v in cumu_bytes.items():
            aggre_bw += v
        aggre_bws.append(aggre_bw)
    print(dname,":",aggre_bws)



get_throughput("multicell_shared_max_no_muting_")
get_throughput("multicell_shared_max_muting_")
get_throughput("multicell_shared_pf_no_muting_")
get_throughput("multicell_shared_pf_muting_")
get_throughput("multicell_all_seperate_pf_no_muting_")
get_throughput("multicell_all_seperate_max_no_muting_")
get_throughput("multicell_macro_micro_pf_no_muting_")
get_throughput("multicell_macro_micro_max_no_muting_")



