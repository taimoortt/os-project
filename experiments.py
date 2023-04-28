import os
# os.system("sudo make clean && sudo make -j8")

for i in range(0,1):
    cmd = f"sudo ./LTE-Sim MultiCell 7 1 10 0 0 1 0 8 1 3 0.1 128 scripts/1slicepf_logs/ue140_b1.json 7 0 {i} > no_fading_multicell_pf{i}.log"
    os.system(cmd)

    cmd = f"sudo ./LTE-Sim MultiCell 7 1 10 0 0 1 0 8 1 3 0.1 128 scripts/1slicepf_logs/ue140_b1.json 1 6 {i} > multicell_macro_micro_pf_no_muting_{i}.log"
    os.system(cmd)

    cmd = f"sudo ./LTE-Sim MultiCell 7 1 10 0 0 1 0 8 1 3 0.1 128 scripts/1slicemax_logs/ue140_b1.json 1 6 {i} > multicell_macro_micro_max_no_muting_{i}.log"
    os.system(cmd)

    cmd = f"sudo ./LTE-Sim MultiCell 7 1 10 0 0 1 0 8 1 3 0.1 128 scripts/1slicepf_logs/ue140_b5.json 1 6 {i} > multicell_shared_pf_muting_{i}.log"
    os.system(cmd)

    cmd = f"sudo ./LTE-Sim MultiCell 7 1 10 0 0 1 0 8 1 3 0.1 128 scripts/1slicemax_logs/ue140_b5.json 1 6 {i} > multicell_shared_max_muting_{i}.log"
    os.system(cmd)