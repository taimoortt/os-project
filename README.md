# MultiCell
![Status](https://img.shields.io/badge/Version-Experimental-green.svg)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

- [MultiCell](#multicell)
  - [How to run MultiCell](#how-to-run-multicell)
    - [Parameters:](#parameters)
    - [Config File:](#config-file)
    - [Note:](#note)
  - [Important Modules](#important-modules)
    - [The centralized scheduler:](#the-centralized-scheduler)
    - [The interference and cqi-with-mute reports:](#the-interference-and-cqi-with-mute-reports)
    - [The logging system:](#the-logging-system)

## How to run MultiCell
After you've built MultiCell, run the following command to start an experiment:

```
${PATH-TO-MultiCell}/LTE-Sim MultiCell [num-of-cells] [radius] [ues-per-cell] 0 0 1 0 [inter-slice scheduler] 1 [mobility speed] 0.1 128 [config file] [random seed]
```

###Parameters:
* num-of-cells: the total number of cells
* radius: the radius of the coverage area of a cell, the default value is 1
* ues-per-cell: number of UEs instantiated for every cell, note the final number of UEs attached to a cell may change due to handover
* inter-slice scheduler: hardcoded as 8(currently only supports RadioSaber)
* mobility speed: the mobility speed of UEs
* config file: a JSON-based configuration file
* random seed: random seed


###Config File:
The config file is in JSON file format. It configures the scheduling algorithm, the number of UEs, the weight of every slice, and how UEs in every slice instantiate applications and flows.

* Currently let's only test backlogged flows, so please keep ```video_app, video_birate, internet_flow, if_bitrate``` all zero
* Make sure that the sum of ```ues_per_slice``` in config file should be equal to ```num-of-cells``` times ```ues-per-cell```
* Set ```enable_comp``` to configure muting behavior
* Set ```enable_tune_weights``` to tune the weight of slices in every cell based on the number of UEs(baseline2)


###Note:
I strongly suggest running the scripts ```./scripts/run_exp.sh``` and checking the logs to understand MultiCell better.

##Important Modules
### The centralized scheduler:
The implementation is in the file: ```./src/componentManagers/FrameManager.cpp```

* Baseline1: ```line 413-421``` calculates the per-cell slice quota; ``` RadioSaberAllocateOneRB()``` is the implemenation
* Baseline2: ```TuneWeightsAcrossCells()``` tune the weight of slices in every cell in the beginning, the rest is same as baseline1
* Baseline3: ```line 422-440``` calculates the slice quota across all cells in a global manner; ```RadioSaberAllocateOneRBGlobal()``` is the implementation
* Baseline5: ```FinalizeAllocation()``` covers the final allocation and muting logic. If cells enable CoMP, some cells are muted based on some logic

### The interference and cqi-with-mute reports:
* Check ```UeLtePhy::StartRx()``` to understand how SINR and SINR-with-mute are calculated
* ```./src/phy/interference.cpp``` shows the logic where the RSRP(Interference) from every cell is calculated

### The logging system:
* Set ```SCHEDULER_DEBUG``` to debug the scheduling parts
* Set ```INTERFERENCE_DEBUG``` to debug the interference and RSRP calculation
