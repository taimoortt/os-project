#include "radiosaber-downlink-scheduler.h"
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
#include <cassert>
#include <unordered_set>

using SchedulerAlgo = SliceContext::SchedulerAlgo;

RadioSaberDownlinkScheduler::RadioSaberDownlinkScheduler(std::string config_fname)
: DownlinkPacketScheduler(config_fname) {
  SetMacEntity (0);
  CreateFlowsToSchedule ();
  slice_rbs_offset_.resize(slice_ctx_.num_slices_, 0);
  slice_target_rbs_.resize(slice_ctx_.num_slices_, 0);
  std::cout << "construct RadioSaber Downlink Scheduler." << std::endl;
}

RadioSaberDownlinkScheduler::~RadioSaberDownlinkScheduler()
{
  Destroy ();
}

void RadioSaberDownlinkScheduler::CalculateSliceQuota()
{
  int nb_rbs = GetMacEntity()->GetDevice()->GetPhy()
    ->GetBandwidthManager()->GetDlSubChannels().size();
  FlowsToSchedule* flows = GetFlowsToSchedule();
  std::unordered_set<int> slice_with_data;
  std::fill(slice_target_rbs_.begin(), slice_target_rbs_.end(), 0);
  int extra_rbs = nb_rbs;
  for (auto it = flows->begin(); it != flows->end(); ++it) {
    assert(*it != nullptr);
    int slice_id = (*it)->GetSliceID();
    if (slice_with_data.find(slice_id) != slice_with_data.end()) {
      continue;
    }
    slice_with_data.insert(slice_id);
    slice_target_rbs_[slice_id] = (int)(nb_rbs * slice_ctx_.weights_[slice_id]
      + slice_rbs_offset_[slice_id]);
    extra_rbs -= slice_target_rbs_[slice_id];
  }
  if (slice_with_data.size() == 0)
    return;
  // we reallocate extra rbs to slices; a random slice gets extra_rbs % num_slice
  int rand_idx = rand() % slice_with_data.size();
  for (auto it = slice_with_data.begin(); it != slice_with_data.end(); it++) {
    int slice_id = *it;
    slice_target_rbs_[slice_id] = extra_rbs / slice_with_data.size();
    if (rand_idx == 0) {
      slice_target_rbs_[slice_id] += extra_rbs % slice_with_data.size();
    }
    rand_idx -= 1;
    // caculate the offset for next TTI
    slice_rbs_offset_[slice_id] = 0; // since we consider rbg in the future
  }
}

double
RadioSaberDownlinkScheduler::ComputeSchedulingMetric(
  RadioBearer *bearer, double spectralEfficiency, int subChannel)
{
  double metric = 0;
  double averageRate = bearer->GetAverageTransmissionRate();
  int slice_id = bearer->GetDestination()->GetSliceID();
  spectralEfficiency = spectralEfficiency * 180000 / 1000; // set the unit to kbps
  averageRate /= 1000.0; // set the unit of both params to kbps
  SchedulerAlgo param = slice_ctx_.algo_params_[slice_id];
  if (param.alpha == 0) {
    metric = pow(spectralEfficiency, param.epsilon) / pow(averageRate, param.psi);
  }
  else {
    if (param.beta) {
      double HoL = bearer->GetHeadOfLinePacketDelay();
      metric = HoL * pow(spectralEfficiency, param.epsilon)
        / pow(averageRate, param.psi);
    }
    else {
      metric = pow(spectralEfficiency, param.epsilon)
        / pow(averageRate, param.psi);
    }
  }
  return metric;
}

