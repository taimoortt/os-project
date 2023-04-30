#!/usr/bin/python3
TIMES = 1
RADIUS = 1000
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
                ues.append((int(float(spl[4])),int(float(spl[6])),int(float(spl[7][:-1])), int(spl[9][:-1]))) # ID, X, Y, Cell ID
            
            elif ('Created Cell,' in i):
                spl = i.split(' ')
                cells.append((int(float(spl[3][:-1])),int(float(spl[5])),int(float(spl[6][:-1])))) # ID, X, Y
            
            elif ('Created Micro,' in i):
                spl = i.split(' ')
                micro.append((int(float(spl[3][:-1])),int(float(spl[5])),int(float(spl[6][:-1])))) # ID, X, Y

        cells_zipped = list(zip(*cells))
        ues_zipped = list(zip(*ues))
        micro_zipped = list(zip(*micro))
        # print(ues_zipped)
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
            circle1 = plt.Circle((i,j), RADIUS, facecolor='none', edgecolor=colors[k])
            plt.gca().add_patch(circle1)

        for i,j,k in zip(micro_zipped[1],micro_zipped[2],micro_zipped[0]):
            plt.scatter(i, j, marker=markers[k], color = colors[k])
            circle1 = plt.Circle((i,j), RADIUS/4, facecolor='none', edgecolor=colors[k])
            plt.gca().add_patch(circle1)

        plt.savefig(dname + "initial.png")


def plot_geo_loc_start_end(dname):
    # Extract Cell Coordinates
    for i in range (TIMES):
        file = open(dname + str(i) + ".log")
        ues_start = []
        ues_end = []
        cells = []
        micro = []

        for i in file.readlines():
            if ('m_idNetworkNode' in i and '150' in i):
                spl = i.split(' ')
                print(spl)
                ues_start.append((int(float(spl[3])),int(float(spl[12])), int(float(spl[15][:-1])), int(float(spl[6])))) # ID, X, Y, Cell ID

            if ('m_idNetworkNode' in i and '2990' in i):
                spl = i.split(' ')
                print(spl)
                ues_end.append((int(float(spl[3])),int(float(spl[12])), int(float(spl[15][:-1])), int(float(spl[6])))) # ID, X, Y, Cell ID
            
            elif ('Created Cell,' in i):
                spl = i.split(' ')
                cells.append((int(float(spl[3][:-1])),int(float(spl[5])),int(float(spl[6][:-1])))) #ID, X, Y
            
            elif ('Created Micro,' in i):
                spl = i.split(' ')
                micro.append((int(float(spl[3][:-1])),int(float(spl[5])),int(float(spl[6][:-1]))))

        cells_zipped = list(zip(*cells))
        ues_start_zipped = list(zip(*ues_start))
        ues_end_zipped = list(zip(*ues_end))
        micro_zipped = list(zip(*micro))
        # print(ues_zipped)
        # print(micro_zipped)
        # print(cells_zipped)

        plt.figure(figsize=(20,12))
        plt.xticks(range(min(ues_start_zipped[1]), 100, max(ues_start_zipped[1])))
        plt.yticks(range(min(ues_start_zipped[2]), 100, max(ues_start_zipped[2])))

        markers = ['x', 'o', 'd', 'p', 'h', 'v', 'o']
        colors = ['red', 'blue', 'green', 'orange', 'magenta', 'maroon', 'turquoise']

        for i,j,k in zip(ues_start_zipped[1],ues_start_zipped[2],ues_start_zipped[3]): # ID, X, Y, Cell
            print(i, '\t', j, '\t', k)
            plt.scatter(i, j, marker=markers[k], color = colors[k])

        for i,j,k in zip(cells_zipped[1],cells_zipped[2],cells_zipped[0]): # X, Y, ID
            plt.scatter(i, j, marker=markers[k], color = colors[k])
            circle1 = plt.Circle((i,j), RADIUS, facecolor='none', edgecolor=colors[k])
            plt.gca().add_patch(circle1)

        for i,j,k in zip(micro_zipped[1],micro_zipped[2],micro_zipped[0]): # X, Y, ID
            plt.scatter(i, j, marker=markers[k], color = colors[k])
            circle1 = plt.Circle((i,j), RADIUS/4, facecolor='none', edgecolor=colors[k])
            plt.gca().add_patch(circle1)

        plt.savefig(dname + "start.png")
        plt.figure(figsize=(20,12))
        plt.xticks(range(min(ues_start_zipped[1]), 100, max(ues_end_zipped[1])))
        plt.yticks(range(min(ues_end_zipped[2]), 100, max(ues_end_zipped[2])))

        markers = ['x', 'o', 'd', 'p', 'h', 'v', 'o']
        colors = ['red', 'blue', 'green', 'orange', 'magenta', 'maroon', 'turquoise']

        for i,j,k in zip(ues_end_zipped[1],ues_end_zipped[2],ues_end_zipped[3]): # ID, X, Y, Cell
            print(i, '\t', j, '\t', k)
            plt.scatter(i, j, marker=markers[k], color = colors[k])

        for i,j,k in zip(cells_zipped[1],cells_zipped[2],cells_zipped[0]): # X, Y, ID
            plt.scatter(i, j, marker=markers[k], color = colors[k])
            circle1 = plt.Circle((i,j), RADIUS, facecolor='none', edgecolor=colors[k])
            plt.gca().add_patch(circle1)

        for i,j,k in zip(micro_zipped[1],micro_zipped[2],micro_zipped[0]): # X, Y, ID
            plt.scatter(i, j, marker=markers[k], color = colors[k])
            circle1 = plt.Circle((i,j), RADIUS/4, facecolor='none', edgecolor=colors[k])
            plt.gca().add_patch(circle1)

        plt.savefig(dname + "end.png")



plot_geo_loc("ue_test_")
plot_geo_loc_start_end("ue_test_")