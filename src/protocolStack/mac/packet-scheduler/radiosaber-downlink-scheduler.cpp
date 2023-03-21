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
#include "../../../utility/eesm-effective-sinr.h"
#include <cassert>
#include <unordered_set>

using SchedulerAlgo = SliceContext::SchedulerAlgo;

RadioSaberDownlinkScheduler::RadioSaberDownlinkScheduler(std::string config_fname)
: DownlinkPacketScheduler(config_fname) {
  SetMacEntity (0);
  CreateFlowsToSchedule ();
  slice_rbs_share_.resize(slice_ctx_.num_slices_, 0);
  slice_rbs_offset_.resize(slice_ctx_.num_slices_, 0);
  slice_rbgs_quota_.resize(slice_ctx_.num_slices_, 0);
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
  std::fill(slice_rbgs_quota_.begin(), slice_rbgs_quota_.end(), 0);
  int extra_rbgs = nb_rbs / RBG_SIZE;
  for (int i = 0; i < slice_ctx_.num_slices_; i++) {
    slice_rbs_share_[i] = nb_rbs * slice_ctx_.weights_[i] + slice_rbs_offset_[i];
    slice_rbgs_quota_[i] = (int)(slice_rbs_share_[i] / RBG_SIZE);
    extra_rbgs -= slice_rbgs_quota_[i];
  }
  int rand_idx = rand() % slice_ctx_.num_slices_;
  slice_rbgs_quota_[rand_idx] += extra_rbgs;
  for (int i = 0; i < slice_ctx_.num_slices_; i++) {
    if (i == rand_idx) {
      slice_rbgs_quota_[i] += extra_rbgs % slice_ctx_.num_slices_;
    }
    slice_rbgs_quota_[i] += extra_rbgs / slice_ctx_.num_slices_;
  }
  // now we reallocate the RBGs of slices with no traffic to slices with traffic
  // step1: find those slices with data
  FlowsToSchedule* flows = GetFlowsToSchedule();
  std::unordered_set<int> slice_with_data;
  extra_rbgs = nb_rbs / RBG_SIZE;
  for (auto it = flows->begin(); it != flows->end(); ++it) {
    assert(*it != nullptr);
    int slice_id = (*it)->GetSliceID();
    if (slice_with_data.find(slice_id) != slice_with_data.end()) {
      continue;
    }
    slice_with_data.insert(slice_id);
    extra_rbgs -= slice_rbgs_quota_[slice_id];
  }
  if (extra_rbgs == 0) return;
  // step2: update slice rbgs quota(set 0 to slice without data, and increase a random slice with extra_rbgs)
  rand_idx = rand() % slice_with_data.size();
  for (int i = 0; i < slice_ctx_.num_slices_; i++) {
    if (slice_with_data.find(i) == slice_with_data.end()) {
      slice_rbgs_quota_[i] = 0;
    }
    else {
      if (rand_idx == 0)
        slice_rbgs_quota_[i] += extra_rbgs;
      rand_idx -= 1;
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

