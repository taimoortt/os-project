#include "nvs-downlink-scheduler.h"
#include "../mac-entity.h"
#include "../../packet/Packet.h"
#include "../../packet/packet-burst.h"
#include "../../../device/NetworkNode.h"
#include "../../../flows/radio-bearer.h"
#include "../../../protocolStack/rrc/rrc-entity.h"
#include "../../../flows/application/Application.h"
#include "../../../device/ENodeB.h"
#include "../../../protocolStack/mac/AMCModule.h"
#include "../../../phy/lte-phy.h"
#include "../../../core/spectrum/bandwidth-manager.h"
#include "../../../core/idealMessages/ideal-control-messages.h"

NVSDownlinkScheduler::NVSDownlinkScheduler(std::string config_fname)
: DownlinkPacketScheduler(config_fname) {
  SetMacEntity (0);
  CreateFlowsToSchedule ();
}

NVSDownlinkScheduler::~NVSDownlinkScheduler()
{
  Destroy ();
}


double
NVSDownlinkScheduler::ComputeSchedulingMetric (RadioBearer *bearer, double spectralEfficiency, int subChannel)
{
  /*
   * For the PF scheduler the metric is computed
   * as follows:
   *
   * metric = spectralEfficiency / averageRate
   */

  double metric = (spectralEfficiency * 180000.)
					  /
					  bearer->GetAverageTransmissionRate();

  return metric;
}

