# Setup
import subprocess
from copy import deepcopy
from math import ceil
from os import listdir, makedirs, path, system
from sys import argv


try:
    import matplotlib.pyplot as plt
except ImportError as ie:
    CHART_GENERATION = False
else:
    CHART_GENERATION = True


HOSTNAMES = [
    "ip-172-31-45-125",
    "ip-172-31-36-169",
    "ip-172-31-36-114",
    "ip-172-31-35-49",
    "ip-172-31-34-34",
    "ip-172-31-32-203",
    "ip-172-31-40-9"
]

HOSTDESCS = [
    "master",
    "slave0",
    "slave1",
    "slave2",
    "slave3",
    "slave4",
    "slave5"
]

DEFAULT_SLOTS_PER_NODE = 2
MAX_PROC = 8
TEST_DIR = path.abspath(path.dirname(__file__))
TEST_FILES_DIR = path.join(TEST_DIR, "files/")
CHARTS_DIR = path.join(TEST_DIR, "charts/")
HOSTFILE = "hosts"

def hostfile(include):
    if include:
        return f"--hostfile {HOSTFILE} "

    return ""

if __name__ == "__main__":

    # Arguments
    n_proc = int(argv[1])
    n_iter = int(argv[2])
    ca_url = argv[3]
    
    if n_proc > MAX_PROC:
        n_proc = MAX_PROC
        print(f"Max numbers of processes allowed: {MAX_PROC}. Launching {n_proc}")

    proc_list = list(range(1, n_proc + 1))

    if not path.exists(TEST_DIR):
        makedirs(TEST_DIR)

    if not path.exists(TEST_FILES_DIR):
        makedirs(TEST_FILES_DIR)

    # Execution
    for n in proc_list:
        n_mach = ceil(n / DEFAULT_SLOTS_PER_NODE)
    
        with open(HOSTFILE, "w") as hf:
            for hn, hd in list(zip(HOSTNAMES, HOSTDESCS))[:n_mach]:
                hf.write(f"{hn:20s} {hd:10s} slots={DEFAULT_SLOTS_PER_NODE}\n")

        command = f"URL={ca_url} mpiexec -n {n} {hostfile(False)}../src/build/krawler_mpi 2> {path.join(TEST_FILES_DIR, f'iters_m{n_mach}_p{n}_i')}"

        for i in range(n_iter):
            command_iter = command + str(i)
            print(f"\nCommand: {command_iter}\n")
            output = subprocess.call(command_iter, shell=True)
    
    # Charts
    if CHART_GENERATION:
        chart_metrics = {
            "TOTAL_PROD_COUNT": [],
            "TOTAL_IDLE_TIME": [],
            "AVG_IDLE_TIME": [],
            "TOTAL_EXEC_TIME": [],
            "AVG_TIME_PER_PRODUCT": [],
        }

        chart_data = {
            str(p): deepcopy(chart_metrics) for p in proc_list
        }

        if not path.exists(CHARTS_DIR):
            makedirs(CHARTS_DIR)

        all_files = listdir(TEST_FILES_DIR)

        for p in proc_list:
            proc_files = [path.join(TEST_FILES_DIR, f) for f in all_files if f"p{p}" in f]
            
            curr_proc_chart_data = deepcopy(chart_metrics)
            for pf in proc_files:
                with open(pf, "r") as iterf:
                    lines = [line.strip("\n") for line in iterf.readlines()]
                    metrics = [l.split(":") for l in lines if l[0] not in "0123456789"]
                    
                for k, v in metrics:
                    curr_proc_chart_data[k].append(float(v))
            
            # print(curr_proc_chart_data)
            for k in curr_proc_chart_data:
                metric = curr_proc_chart_data[k]
                chart_data[str(p)][k] = (sum(metric) / len(metric) if len(metric) != 0 else 0)

        plot_data = deepcopy(chart_metrics)
        for p in chart_data:
            for k in chart_data[p]:
                plot_data[k].append(chart_data[p][k])

        plot_data["PROCESSES"] = proc_list[:]

        for k in plot_data:
            if k != "PROCESSES":
                fig = plt.figure()
                plt.title(k)
                plt.xticks(plot_data["PROCESSES"])
                plt.plot(plot_data["PROCESSES"], plot_data[k], "o-")
                plt.savefig(path.join(CHARTS_DIR, k)+".png")
                plt.close(fig)
            
