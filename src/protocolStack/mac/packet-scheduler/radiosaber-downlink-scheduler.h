#ifndef NVSDOWNLINKSCHEDULER_H_
#define NVSDOWNLINKSCHEDULER_H_

#include "downlink-packet-scheduler.h"

class RadioSaberDownlinkScheduler : public DownlinkPacketScheduler {
private:
// the ewma beta for inter-slice scheduling
  const double beta_ = 0.01;
  int current_slice_ = -1;

public:
	RadioSaberDownlinkScheduler(std::string config_fname);
	virtual ~RadioSaberDownlinkScheduler();

	virtual double ComputeSchedulingMetric (RadioBearer *bearer, double spectralEfficiency, int subChannel);
};

#endif /* DLPFPACKETSCHEDULER_H_ */
