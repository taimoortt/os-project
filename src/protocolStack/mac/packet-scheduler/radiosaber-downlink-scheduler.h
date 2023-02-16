#ifndef RADIOSABERDOWNLINKSCHEDULER_H_
#define RADIOSABERDOWNLINKSCHEDULER_H_

#include "downlink-packet-scheduler.h"
#include <vector>

class RadioSaberDownlinkScheduler : public DownlinkPacketScheduler {
private:
// the ewma beta for inter-slice scheduling
  const double beta_ = 0.01;
  int current_slice_ = -1;
  

public:
	RadioSaberDownlinkScheduler(std::string config_fname);
	virtual ~RadioSaberDownlinkScheduler();
	virtual double ComputeSchedulingMetric (RadioBearer *bearer, double spectralEfficiency, int subChannel);
  
  std::vector<double> slice_rbs_offset_;
  std::vector<int> slice_target_rbs_;
  void CalculateSliceQuota();
};

#endif
