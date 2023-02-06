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
  std::vector<bool> slice_with_data(slice_ctx_.num_slices_, false);
  std::fill(slice_target_rbs_.begin(), slice_target_rbs_.end(), 0);
  int num_nonempty_slices = 0;
  int extra_rbs = nb_rbs;
  for (auto it = flows->begin(); it != flows->end(); ++it) {
    assert(*it != nullptr);
    int slice_id = (*it)->GetSliceID();
    if (slice_with_data[slice_id])
      continue;
    num_nonempty_slices += 1;
    slice_with_data[slice_id] = true;
    slice_target_rbs_[slice_id] = (int)(nb_rbs * slice_ctx_.weights_[slice_id]
      + slice_rbs_offset_[slice_id]);
    extra_rbs -= slice_target_rbs_[slice_id];
  }
  if (num_nonempty_slices == 0)
    return;
  // we enable reallocation between slices, but not flows
  bool is_first_slice = true;
  int rand_begin_idx = rand();
  for (int i = 0; i < slice_ctx_.num_slices_; ++i) {
    int k = (i + rand_begin_idx) % slice_ctx_.num_slices_;
    if (slice_with_data[k]) {
      slice_target_rbs_[k] += extra_rbs / num_nonempty_slices;
      if (is_first_slice) {
        slice_target_rbs_[k] += extra_rbs % num_nonempty_slices;
        is_first_slice = false;
      }
    }
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

