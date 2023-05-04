import statistics as stats
import matplotlib.pyplot as plt
import math
import numpy as np
dname = "ue_test_"
plt.figure(figsize=(10,6))

def get_ue_sinr_dist(dname, raw = 0):
    cells = {}
    ues = {}
    all_eff_sinr = {}
    all_sinr = []
    all_tbs = {}
    all_tbs_ng = []
    file = open(dname)
    for i in file.readlines():
        spl = i.split(' ')
        if ('Created Cell,' in i or 'Created Micro,' in i):
            cell_id = int(float(spl[3][:-1]))
            x = int(float(spl[5]))
            y = int(float(spl[6][:-1]))
            cells[cell_id] = [x,y]
        elif ('m_idNetworkNode' in i):
            ue_id = int(float(spl[3]))
            x = int(float(spl[12]))
            y = int(float(spl[15][:-1]))
            cell_id = int(spl[6])
            ues[ue_id] = [cell_id, x, y]
        elif ('eff_sinr' in i):
            ue_id = int(spl[10])
            if(spl[12] == 'inf'):
                eff_sinr = 30
            else:
                eff_sinr = float(spl[12])
            tbs = int(spl[14][:-1])/1000 # kbps -> Mbps
            if(eff_sinr > 34.6 or eff_sinr < -4.63):
                continue
            all_sinr.append(eff_sinr)
            all_tbs_ng.append(tbs)
            try:
                all_eff_sinr[ue_id].append(eff_sinr)
                all_tbs[ue_id].append(tbs)
            except:
                all_eff_sinr[ue_id] = [eff_sinr]
                all_tbs[ue_id] = [tbs]
    if(raw):
        return all_sinr, all_tbs_ng

    # Aggregating the UE TBS and SINR over the simulation
    for i in ues.keys():
        mean_sinr = stats.mean(all_eff_sinr[i])
        mean_tbs = stats.mean(all_tbs[i])
        ues[i].append(mean_sinr)
        ues[i].append(mean_tbs)

    return cells, ues

def plot_sinr_distance(dname):
    cells, ues = get_ue_sinr_dist(dname+'0.log')
    cells_muted, ues_muted = get_ue_sinr_dist(dname+'muting_0.log')
    dist_sinr = {}
    for i in ues.values():
        # Calculate distance from Cell
        cell_coordinates = cells[i[0]]
        ue_coordinates = [i[1], i[2]]
        abs_distance = math.sqrt(math.pow(cell_coordinates[0]-ue_coordinates[0],2) + math.pow(cell_coordinates[0]-ue_coordinates[0],2))
        dist_sinr[abs_distance] = i[3]

    for dist, sinr in dist_sinr.items():
        plt.scatter(dist,sinr)
        plt.xlabel('Distance')
        plt.ylabel('Mean SINR')
    plt.title('Relation between Distance and SINR - Without Muting')
    plt.savefig(dname+'scatter_plot_dist_sinr.png')
    plt.clf()

    dist_sinr = {}
    for i in ues_muted.values():
        # Calculate distance from Cell
        cell_coordinates = cells[i[0]]
        ue_coordinates = [i[1], i[2]]
        abs_distance = math.sqrt(math.pow(cell_coordinates[0]-ue_coordinates[0],2) + math.pow(cell_coordinates[0]-ue_coordinates[0],2))
        dist_sinr[abs_distance] = i[3]

    for dist, sinr in dist_sinr.items():
        plt.scatter(dist,sinr)
        plt.xlabel('Distance')
        plt.ylabel('Mean SINR')
    plt.title('Relation between Distance and SINR - With Muting')
    plt.savefig(dname+'scatter_plot_dist_sinr_mute.png')
    plt.clf()

def plot_sinr_tbs(dname):
    cells, ues = get_ue_sinr_dist(dname+'0.log')
    cells_muted, ues_muted = get_ue_sinr_dist(dname+'muting_0.log')

    sinr_dr = {}
    for i in ues.values():
        sinr_dr[i[3]] = i[4]
    for sinr, dr in sinr_dr.items():
        plt.scatter(sinr,dr)
        plt.xlabel('Mean SINR')
        plt.ylabel('Mean TBS Size (Mbps)')
    plt.title('Relation between SINR and TBS size - Without Muting')
    plt.savefig(dname+'scatter_plot_sinr_dr.png')
    plt.clf()

    sinr_dr = {}
    for i in ues_muted.values():
        sinr_dr[i[3]] = i[4]
    for sinr, dr in sinr_dr.items():
        plt.scatter(sinr,dr)
        plt.xlabel('Mean SINR')
        plt.ylabel('Mean TBS Size (Mbps)')
    plt.title('Relation between SINR and TBS size - With Muting')
    plt.savefig(dname+'scatter_plot_sinr_dr_mute.png')
    plt.clf()

def plot_cdf_sinr_agg(dname):
    cells, ues = get_ue_sinr_dist(dname + '0.log')
    cells_muted, ues_muted = get_ue_sinr_dist(dname+'muting_0.log')
    N = len(ues)
    ues_sinr = []
    ues_sinr_muted = []
    for i in ues.values():
        ues_sinr.append(i[3])
    for i in ues_muted.values():
        ues_sinr_muted.append(i[3])
    ues_sinr_sorted = np.sort(ues_sinr)
    ues_sinr_muted_sorted = np.sort(ues_sinr_muted)

    y = np.arange(N)/float(N)
    plt.plot(ues_sinr_sorted, y, label='Not Muted')
    plt.plot(ues_sinr_muted_sorted, y, label='Muted')
    plt.legend(fontsize = 12)
    plt.title('CDF of per UE SINR aggregated through the simulation')
    plt.xlabel('SINR')
    plt.savefig(dname+'cdf_sinr.png')
    plt.clf()

def plot_cdf_dr_agg(dname):
    cells, ues = get_ue_sinr_dist(dname + '0.log')
    cells_muted, ues_muted = get_ue_sinr_dist(dname+'muting_0.log')
    N = len(ues)
    ues_sinr = []
    ues_sinr_muted = []
    for i in ues.values():
        ues_sinr.append(i[3])
    for i in ues_muted.values():
        ues_sinr_muted.append(i[4])
    ues_sinr_sorted = np.sort(ues_sinr)
    ues_sinr_muted_sorted = np.sort(ues_sinr_muted)
    y = np.arange(N)/float(N)
    plt.plot(ues_sinr_sorted, y, label='Not Muted')
    plt.plot(ues_sinr_muted_sorted, y, label='Muted')
    plt.legend(fontsize = 12)
    plt.title('CDF of per UE TBS aggregated through the simulation (Mbps)')
    plt.savefig(dname+'cdf_dr.png')
    plt.xlabel('TBS Size (Mbps)')
    plt.clf()
    
def plot_cdf_sinr(dname):
    all_eff_sinr, _ = get_ue_sinr_dist(dname + '0.log', 1)
    all_eff_sinr_muted, _ = get_ue_sinr_dist(dname+'muting_0.log', 1)
    all_micro_eff_sinr, _ = get_ue_sinr_dist(dname + 'micro_0.log', 1)
    all_micro_eff_sinr_muted, _ = get_ue_sinr_dist(dname + 'micro_muting_0.log', 1)


    # N = len(all_eff_sinr)
    # ues_sinr_sorted = np.sort(all_eff_sinr)
    # y = np.arange(N)/float(N)
    # plt.plot(ues_sinr_sorted, y, label='Heterogeneous - Not Muted')

    N = len(all_eff_sinr_muted)
    ues_sinr_muted_sorted = np.sort(all_eff_sinr_muted)
    y = np.arange(N)/float(N)
    plt.plot(ues_sinr_muted_sorted, y, label='Heterogeneous - Muted')

    # N = len(all_micro_eff_sinr)
    # micro_ues_sinr_sorted = np.sort(all_micro_eff_sinr)
    # y = np.arange(N)/float(N)
    # plt.plot(micro_ues_sinr_sorted, y, label='Micro only - Not Muted')

    N = len(all_micro_eff_sinr_muted)
    micro_ues_sinr_muted_sorted = np.sort(all_micro_eff_sinr_muted)
    y = np.arange(N)/float(N)
    plt.plot(micro_ues_sinr_muted_sorted, y, label='Micro only - Muted')

    plt.legend(fontsize = 12)
    plt.title('CDF of per UE SINR per subframe - No Aggregation')
    plt.xlabel('SINR')
    plt.savefig(dname+'cdf_sinr_nonagg.png')
    plt.clf()

def plot_cdf_dr(dname):
    _, all_tbs = get_ue_sinr_dist(dname + '0.log', 1)
    _, all_tbs_muted = get_ue_sinr_dist(dname+'muting_0.log', 1)

    N = len(all_tbs)
    y = np.arange(N)/float(N)
    ues_tbs_sorted = np.sort(all_tbs)
    plt.plot(ues_tbs_sorted, y, label='Not Muted')

    N = len(all_tbs_muted)
    y = np.arange(N)/float(N)
    ues_tbs_muted_sorted = np.sort(all_tbs_muted)
    plt.plot(ues_tbs_muted_sorted, y, label='Muted')

    plt.legend(fontsize = 12)
    plt.title('CDF of per UE TBS per subframe - No Aggregation')
    plt.xlabel('TBS (Mbps)')
    plt.savefig(dname+'cdf_dr_nonagg.png')
    plt.clf()

# plot_sinr_distance(dname)
# plot_sinr_tbs(dname)
# plot_cdf_dr(dname)
plot_cdf_sinr(dname)
# plot_cdf_dr_agg(dname)
# plot_cdf_sinr_agg(dname)