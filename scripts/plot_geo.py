#!/usr/bin/python3
import matplotlib.pyplot as plt

file = open('res.txt')

# Extract Cell Coordinates
ues = []
cells = []

for i in file.readlines():
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

plt.figure(figsize=(20,12))
plt.xticks(range(min(ues_zipped[1]), 100, max(ues_zipped[1])))
plt.yticks(range(min(ues_zipped[2]), 100, max(ues_zipped[2])))

plt.scatter(ues_zipped[1], ues_zipped[2], marker="X")
plt.scatter(cells_zipped[1], cells_zipped[2], marker = 's')
for i in range(len(cells_zipped[0])):
    circle1 = plt.Circle((cells_zipped[1][i], cells_zipped[2][i]), 1000, facecolor="none",edgecolor = 'black')
    plt.gca().add_patch(circle1)

plt.savefig('res.png')


print(len(cells_zipped[1]))
print()
print(len(ues_zipped[1]))
