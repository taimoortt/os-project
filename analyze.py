import matplotlib.pyplot as plt
import statistics as stats
import numpy as np

''' ----- Visualize Cell Placement ----- '''

file = open('res.txt')

# Extract Cell Coordinates
ues = []
cells = []
all_lines = file.readlines()

for i in all_lines:
  if ('Created UE' in i):
      spl = i.split(' ')
      # ues.append((spl[4],spl[6],spl[7][:-1]))
      ues.append((int(float(spl[4])),int(float(spl[6])),int(float(spl[7][:-1]))))
  elif ('Created Cell' in i):
      spl = i.split(' ')
      # cells.append((spl[3][:-1],spl[5],spl[6][:-1]))
      cells.append((int(float(spl[3][:-1])),int(float(spl[5])),int(float(spl[6][:-1]))))

cells_zipped = list(zip(*cells))
ues_zipped = list(zip(*ues))

# plt.figure(figsize=(20,12))
# plt.xticks(range(min(ues_zipped[1]), 100, max(ues_zipped[1])))
# plt.yticks(range(min(ues_zipped[2]), 100, max(ues_zipped[2])))

# plt.scatter(ues_zipped[1], ues_zipped[2], marker="X")
# plt.scatter(cells_zipped[1], cells_zipped[2], marker = 's')
# plt.title("User Distribution - Half Cell Edge", fontsize=19)
# for i in range(len(cells_zipped[0])):
#     circle1 = plt.Circle((cells_zipped[1][i], cells_zipped[2][i]), 1000, facecolor="none",edgecolor = 'black')
#     plt.gca().add_patch(circle1)

# plt.savefig('cell_placement.png')


''' ----- Extract the Effective SINRs and TBS sizes for each user ----- '''
flow_reports = []
eff_sinrs = {}
all_eff_sinrs = []
tbs_sizes = {}
for i in all_lines:
  if('eff_sinr' in i):
    info = i.split(" ")
    id = int(info[1])
    tbs = int(info[11][:-1])
    try:
      eff_sinr = int(float(info[9]))
      all_eff_sinrs.append(eff_sinr)
    except:
      eff_sinr = 30
      all_eff_sinrs.append(eff_sinr)

    # Store all eff_sinrs
    if (id in eff_sinrs.keys()):
      eff_sinrs[id].append(eff_sinr)
    else:
      eff_sinrs[id]=[eff_sinr]

    # Store all tbs
    if (id in tbs_sizes.keys()):
      tbs_sizes[id]+= tbs
    else:
      tbs_sizes[id] = tbs
    # print(i.split(" ")[1], i.split(" ")[8], i.split(" ")[9], i.split(" ")[10], i.split(" ")[11])
# file.close()



''' ----- Make a Bar Plot out of the Effective SINRs ----- '''
x = []
y = []

for i in sorted(eff_sinrs.keys()):
  x.append(i)
  y.append(int(sum(eff_sinrs[i])/9.5))

# plt.figure(figsize=(20,12))
# plt.bar(x,y)
# plt.xlabel("UE ID", fontsize=19)
# plt.ylabel("Average Transport Block Size across 10sec", fontsize=19)
# plt.title("Random User Distribution, Muting Enabled", fontsize=19)
# plt.savefig('bar_plot_eff_sinr.png')

file = open('stat_file.txt', 'a+')
file.write(str(stats.median(y)))
file.write('\t\t')
file.write(str(stats.mean(y)))
file.write('\n****\n')
file.close()




''' ----- Plot a CDF for the Effective SINRs ----- '''
# N = len(all_eff_sinrs)
# x = sorted(all_eff_sinrs)
# y = np.arange(N)/float(N)
# plt.figure(figsize=(30,12))




plt.xlabel('Effective SINR', fontsize = 19)
plt.ylabel('CDF', fontsize = 19)
plt.title('Random User Distribution, Muting Enabled', fontsize = 19)
plt.plot(x, y, marker='o')
plt.savefig('cdf_eff_sinr.png')
