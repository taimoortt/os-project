import os
# os.system("make clean")
# os.system("make")
file = open("stat_file.txt", 'a+')
file.write("\nMuting with Cell Edge Distribution\n")
file.close()

for i in range(1,10):
    os.system("sudo ./LTE-Sim MultiCell 7 1 10 0 0 1 0 8 1 30 0.1 128 scripts/baseline5.json >res.txt")
    os.system("python3 analyze.py")
    # os.system("sudo mv cell_placement.png cell_placement_cell_edge_muting_" + str(i) + ".png")
    # os.system("sudo mv bar_plot_eff_sinr.png bar_plot_eff_sinr_cell_edge_muting_" + str(i) + ".png")
    # os.system("sudo mv cdf_eff_sinr.png cdf_eff_sinr_cell_edge_muting_" + str(i) + ".png")

file = open("stat_file.txt", 'a+')
file.write("\n\n\nNo Muting with Cell Edge Distribution\n")
file.close()

for i in range(1,10):
    os.system("sudo ./LTE-Sim MultiCell 7 1 10 0 0 1 0 8 1 30 0.1 128 scripts/baseline1.json >res.txt")
    os.system("python3 analyze.py")
    # os.system("sudo mv cell_placement.png cell_placement_cell_edge_no_muting_" + str(i) + ".png")
    # os.system("sudo mv bar_plot_eff_sinr.png bar_plot_eff_sinr_cell_edge_no_muting_" + str(i) + ".png")
    # os.system("sudo mv cdf_eff_sinr.png cdf_eff_sinr_cell_edge_no_muting_" + str(i) + ".png")