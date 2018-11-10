import matplotlib.pyplot as plt
from copy import deepcopy
from os import path, listdir
from sys import argv

TEST_DIR = path.abspath(path.dirname(__file__))
TEST_FILES_DIR = path.join(TEST_DIR, "files/")
CHARTS_DIR = path.join(TEST_DIR, "charts/")

proc_list = list(range(1, int(argv[1]) + 1))

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

            metrics = []
            for l in lines:
                if l not in ("", "\n"):
                    if l[0] not in "0123456789":
                        metrics.append(l.split(":"))
            
        for k, v in metrics:
            curr_proc_chart_data[k].append(float(v))
            
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
