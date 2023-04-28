#!/usr/bin/python3
TIMES = 1
import matplotlib.pyplot as plt

def plot_geo_loc(dname):
    # Extract Cell Coordinates
    for i in range (TIMES):
        file = open(dname + str(i) + ".log")
        ues = []
        cells = []
        micro = []

        for i in file.readlines():
            if ('Created UE' in i):
                spl = i.split(' ')
                ues.append((int(float(spl[4])),int(float(spl[6])),int(float(spl[7][:-1])), int(spl[9][:-1])))
            
            elif ('Created Cell,' in i):
                spl = i.split(' ')
                print(spl)
                cells.append((int(float(spl[3][:-1])),int(float(spl[5])),int(float(spl[6][:-1]))))
            
            elif ('Created Micro,' in i):
                spl = i.split(' ')
                micro.append((int(float(spl[3][:-1])),int(float(spl[5])),int(float(spl[6][:-1]))))

        cells_zipped = list(zip(*cells))
        ues_zipped = list(zip(*ues))
        micro_zipped = list(zip(*micro))
        print(ues_zipped)
        # print(micro_zipped)
        # print(cells_zipped)

        plt.figure(figsize=(20,12))
        plt.xticks(range(min(ues_zipped[1]), 100, max(ues_zipped[1])))
        plt.yticks(range(min(ues_zipped[2]), 100, max(ues_zipped[2])))

        markers = ['x', 'o', 'd', 'p', 'h', 'v', 'o']
        colors = ['red', 'blue', 'green', 'orange', 'magenta', 'maroon', 'turquoise']

        for i,j,k in zip(ues_zipped[1],ues_zipped[2],ues_zipped[3]):
            plt.scatter(i, j, marker=markers[k], color = colors[k])

        for i,j,k in zip(cells_zipped[1],cells_zipped[2],cells_zipped[0]):
            plt.scatter(i, j, marker=markers[k], color = colors[k])
            circle1 = plt.Circle((i,j), 1000, facecolor='none', edgecolor=colors[k])
            plt.gca().add_patch(circle1)

        # for i,j,k in zip(micro_zipped[1],micro_zipped[2],micro_zipped[0]):
        #     plt.scatter(i, j, marker=markers[k], color = colors[k])
        #     circle1 = plt.Circle((i,j), 300, facecolor='none', edgecolor=colors[k])
        #     plt.gca().add_patch(circle1)

        # for i in range(len(cells_zipped[0])):
        #     circle1 = plt.Circle((cells_zipped[1][i], cells_zipped[2][i]), 1000, facecolor="none",edgecolor = 'red')
        #     plt.gca().add_patch(circle1)

        # for i in range(len(micro_zipped[0])):
        #     circle1 = plt.Circle((micro_zipped[1][i], micro_zipped[2][i]), 300, facecolor="none",edgecolor = 'blue')
        #     plt.gca().add_patch(circle1)

        plt.savefig(dname + ".png")

plot_geo_loc("fading_1_cell_max_")


# plot_geo_loc("multicell_all_seperate_max_no_muting_")
# plot_geo_loc("multicell_shared_max_no_muting_")
# plot_geo_loc("multicell_shared_max_muting_")
# plot_geo_loc("multicell_shared_pf_no_muting_")
# plot_geo_loc("multicell_shared_pf_muting_")
# plot_geo_loc("multicell_all_seperate_pf_no_muting_")
# plot_geo_loc("multicell_all_seperate_max_no_muting_")
# plot_geo_loc("multicell_macro_micro_pf_no_muting_")
# plot_geo_loc("multicell_macro_micro_max_no_muting_")
