#ifndef NVSDOWNLINKSCHEDULER_H_
#define NVSDOWNLINKSCHEDULER_H_

#include "downlink-packet-scheduler.h"

class NVSDownlinkScheduler : public DownlinkPacketScheduler {
public:
	NVSDownlinkScheduler(std::string config_fname);
	virtual ~NVSDownlinkScheduler();

	virtual double ComputeSchedulingMetric (RadioBearer *bearer, double spectralEfficiency, int subChannel);
};

#endif /* DLPFPACKETSCHEDULER_H_ */
