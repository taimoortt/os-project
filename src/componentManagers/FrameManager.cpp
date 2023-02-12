/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010,2011,2012,2013 TELEMATICS LAB, Politecnico di Bari
 *
 * This file is part of LTE-Sim
 *
 * LTE-Sim is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation;
 *
 * LTE-Sim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LTE-Sim; if not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Giuseppe Piro <g.piro@poliba.it>
 */



#include "FrameManager.h"
#include "../load-parameters.h"
#include "../device/ENodeB.h"
#include "../device/HeNodeB.h"
#include "../protocolStack/mac/packet-scheduler/downlink-packet-scheduler.h"
#include "../protocolStack/mac/AMCModule.h"
#include "../core/spectrum/bandwidth-manager.h"
#include "../utility/eesm-effective-sinr.h"
#include "../flows/radio-bearer.h"
#include "../flows/application/Application.h"
#include "../phy/enb-lte-phy.h"
#include <cassert>
#include <unordered_set>
#include "../protocolStack/mac/packet-scheduler/radiosaber-downlink-scheduler.h"

FrameManager* FrameManager::ptr=NULL;

FrameManager::FrameManager() {
  m_nbFrames = 0;
  m_nbSubframes = 0;
  m_TTICounter = 0;
  m_frameStructure = FrameManager::FRAME_STRUCTURE_FDD; //Default Value
  m_TDDFrameConfiguration = 1; //Default Value
  Simulator::Init()->Schedule(0.0, &FrameManager::Start, this);
}

FrameManager::~FrameManager()
{}

void
FrameManager::SetFrameStructure (FrameManager::FrameStructure frameStructure)
{
  m_frameStructure = frameStructure;
}

FrameManager::FrameStructure
FrameManager::GetFrameStructure (void) const
{
  return m_frameStructure;
}


void
FrameManager::SetTDDFrameConfiguration (int configuration)
{
  if (configuration < 0 && configuration > 6)
    {
	  m_TDDFrameConfiguration = 0; //Default Value
    }
  else
    {
	  m_TDDFrameConfiguration = configuration;
    }
}

int
FrameManager::GetTDDFrameConfiguration (void) const
{
  return m_TDDFrameConfiguration;
}

int
FrameManager::GetSubFrameType (int nbSubFrame)
{
  return TDDConfiguration [GetTDDFrameConfiguration ()][nbSubFrame-1];
}


void
FrameManager::UpdateNbFrames (void)
{
  m_nbFrames++;
}

int
FrameManager::GetNbFrames (void) const
{
  return m_nbFrames;
}

void
FrameManager::UpdateNbSubframes (void)
{
  m_nbSubframes++;
}

void
FrameManager::ResetNbSubframes (void)
{
  m_nbSubframes = 0;
}

int
FrameManager::GetNbSubframes (void) const
{
  return m_nbSubframes;
}

void
FrameManager::UpdateTTIcounter (void)
{
  m_TTICounter++;
}

unsigned long
FrameManager::GetTTICounter () const
{
  return m_TTICounter;
}

void
FrameManager::Start (void)
{
#ifdef FRAME_MANAGER_DEBUG
  std::cout << " LTE Simulation starts now! "<< std::endl;
#endif

  Simulator::Init()->Schedule(0.0, &FrameManager::StartFrame, this);
}

void
FrameManager::StartFrame (void)
{
  UpdateNbFrames ();

#ifdef FRAME_MANAGER_DEBUG
  std::cout << " +++++++++ Start Frame, time =  "
      << Simulator::Init()->Now() << " +++++++++" << std::endl;
#endif

  Simulator::Init()->Schedule(0.0,
							  &FrameManager::StartSubframe,
							  this);
}

void
FrameManager::StopFrame (void)
{
  Simulator::Init()->Schedule(0.0,
							  &FrameManager::StartFrame,
							  this);
}

void
FrameManager::StartSubframe (void)
{
#ifdef FRAME_MANAGER_DEBUG
  std::cout << " --------- Start SubFrame, time =  "
      << Simulator::Init()->Now() << " --------- " << std::endl;
#endif

  UpdateTTIcounter ();
  UpdateNbSubframes ();

  /*
   * At the beginning of each sub-frame the simulation
   * update user position. This function could be
   * implemented also every TTI.
   */
  //UpdateUserPosition(); --> moved to each UE class


#ifdef PLOT_USER_POSITION
  NetworkManager::Init()->PrintUserPosition();
#endif

  /*
   * According to the Frame Structure, the DW/UL link scheduler
   * will be called for each sub-frame.
   * (RBs allocation)
   */
  Simulator::Init()->Schedule(0, &FrameManager::CentralResourceAllocation, this);
  Simulator::Init()->Schedule(0.001,
							  &FrameManager::StopSubframe,
							  this);
}

void
FrameManager::StopSubframe (void)
{
  if (GetNbSubframes () == 10)
    {
	  ResetNbSubframes ();
	  Simulator::Init()->Schedule(0.0,
								  &FrameManager::StopFrame,
								  this);
    }
  else
    {
	  Simulator::Init()->Schedule(0.0,
								  &FrameManager::StartSubframe,
								  this);
    }
}


NetworkManager*
FrameManager::GetNetworkManager (void)
{
  return NetworkManager::Init();
}

void
FrameManager::UpdateUserPosition(void)
{
  GetNetworkManager ()->UpdateUserPosition (GetTTICounter () / 1000.0);
}


void
FrameManager::ResourceAllocation(void)
{
  std::vector<ENodeB*> *records = GetNetworkManager ()->GetENodeBContainer ();
  std::vector<ENodeB*>::iterator iter;
  ENodeB *record;
  for (iter = records->begin (); iter != records->end (); iter++)
	{
	  record = *iter;

#ifdef FRAME_MANAGER_DEBUG
	  std::cout << " FRAME_MANAGER_DEBUG: RBs allocation for eNB " <<
		  record->GetIDNetworkNode() << std::endl;
#endif


	  if (GetFrameStructure () == FrameManager::FRAME_STRUCTURE_FDD)
		{
		  //record->ResourceBlocksAllocation ();
		  Simulator::Init()->Schedule(0.0, &ENodeB::ResourceBlocksAllocation,record);
		}
	  else
		{
		  //see frame configuration in TDDConfiguration
		  if (GetSubFrameType (GetNbSubframes ()) == 0)
			{
#ifdef FRAME_MANAGER_DEBUG
			  std::cout << " FRAME_MANAGER_DEBUG: SubFrameType = "
				  "	SUBFRAME_FOR_DOWNLINK " << std::endl;
#endif
			  //record->DownlinkResourceBlockAllocation();
			  Simulator::Init()->Schedule(0.0, &ENodeB::DownlinkResourceBlockAllocation,record);
			}
		  else if(GetSubFrameType (GetNbSubframes ()) == 1)
			{
#ifdef FRAME_MANAGER_DEBUG
			  std::cout << " FRAME_MANAGER_DEBUG: SubFrameType = "
				  "	SUBFRAME_FOR_UPLINK " << std::endl;
#endif
			  //record->UplinkResourceBlockAllocation();
			  Simulator::Init()->Schedule(0.0, &ENodeB::UplinkResourceBlockAllocation,record);
			}
		  else
			{
#ifdef FRAME_MANAGER_DEBUG
			  std::cout << " FRAME_MANAGER_DEBUG: SubFrameType = "
				  "	SPECIAL_SUBFRAME" << std::endl;
#endif
			}
		}
	}

  std::vector<HeNodeB*> *records_2 = GetNetworkManager ()->GetHomeENodeBContainer();
    std::vector<HeNodeB*>::iterator iter_2;
    HeNodeB *record_2;
    for (iter_2 = records_2->begin (); iter_2 != records_2->end (); iter_2++)
  	{
  	  record_2 = *iter_2;

  #ifdef FRAME_MANAGER_DEBUG
  	  std::cout << " FRAME_MANAGER_DEBUG: RBs allocation for eNB " <<
  		  record_2->GetIDNetworkNode() << std::endl;
  #endif


  	  if (GetFrameStructure () == FrameManager::FRAME_STRUCTURE_FDD)
  		{
  		  //record_2->ResourceBlocksAllocation ();
  		  Simulator::Init()->Schedule(0.0, &ENodeB::ResourceBlocksAllocation,record_2);
  		}
  	  else
  		{
  		  //see frame configuration in TDDConfiguration
  		  if (GetSubFrameType (GetNbSubframes ()) == 0)
  			{
  #ifdef FRAME_MANAGER_DEBUG
  			  std::cout << " FRAME_MANAGER_DEBUG: SubFrameType = "
  				  "	SUBFRAME_FOR_DOWNLINK " << std::endl;
  #endif
  			  //record_2->DownlinkResourceBlockAllocation();
  			  Simulator::Init()->Schedule(0.0, &ENodeB::DownlinkResourceBlockAllocation,record_2);
  			}
  		  else if(GetSubFrameType (GetNbSubframes ()) == 1)
  			{
  #ifdef FRAME_MANAGER_DEBUG
  			  std::cout << " FRAME_MANAGER_DEBUG: SubFrameType = "
  				  "	SUBFRAME_FOR_UPLINK " << std::endl;
  #endif
  			  //record_2->UplinkResourceBlockAllocation();
  			  Simulator::Init()->Schedule(0.0, &ENodeB::UplinkResourceBlockAllocation,record_2);
  			}
  		  else
  			{
  #ifdef FRAME_MANAGER_DEBUG
  			  std::cout << " FRAME_MANAGER_DEBUG: SubFrameType = "
  				  "	SPECIAL_SUBFRAME" << std::endl;
  #endif
  			}
  		}
  	}
}

void
FrameManager::CentralResourceAllocation(void)
{
  std::vector<ENodeB*> *enodebs = GetNetworkManager ()->GetENodeBContainer ();
  assert(GetFrameStructure() == FrameManager::FRAME_STRUCTURE_FDD);
#ifdef FRAME_MANAGER_DEBUG
	std::cout << "Resource Allocation for eNB:";
  for (auto iter = enodebs->begin (); iter != enodebs->end (); iter++) {
	  ENodeB* enb = *iter;
    std::cout << " " << enb->GetIDNetworkNode();
	}
  std::cout << std::endl;
#endif

#ifdef SET_CENTRAL_SCHEDULER
  std::cout << "Central Downlink RBs Allocation!" << std::endl;
  CentralDownlinkRBsAllocation();
#else
  std::cout << "Original Per-Cell Downlink RBs Allocation!" << std::endl;
for (auto iter = enodebs->begin (); iter != enodebs->end (); iter++) {
	  ENodeB* enb = *iter;
    enb->DownlinkResourceBlockAllocation();
	}
#endif
  for (auto iter = enodebs->begin (); iter != enodebs->end (); iter++) {
	  ENodeB* enb = *iter;
    enb->UplinkResourceBlockAllocation();
	}
}

void
FrameManager::CentralDownlinkRBsAllocation(void)
{
  std::vector<ENodeB*> *enodebs =
    GetNetworkManager ()->GetENodeBContainer ();
  std::vector<DownlinkPacketScheduler*> schedulers;
  // initialization
  for (auto it = enodebs->begin(); it != enodebs->end(); it++) {
    ENodeB* enb = *it;
    DownlinkPacketScheduler* scheduler =
      (DownlinkPacketScheduler*)enb->GetDLScheduler();
    assert(scheduler != NULL);
    schedulers.push_back(scheduler);
  }
  // set up schedulers
  bool available_flow = false;
  for (int j = 0; j < schedulers.size(); j++) {
    schedulers[j]->UpdateAverageTransmissionRate();
    schedulers[j]->SelectFlowsToSchedule();
    if (schedulers[j]->GetFlowsToSchedule()->size() > 0) {
      available_flow = true;
    }
  }
  // if no flow is available in any slice, stop schedulers
  if (!available_flow) {
    for (int j = 0; j < schedulers.size(); j++)
      schedulers[j]->StopSchedule();
    return;
  }
  // Assume that every eNB has same number of RBs
  int nb_of_rbs = schedulers[0]->GetMacEntity()->GetDevice()->GetPhy()
      ->GetBandwidthManager()->GetDlSubChannels().size ();
  bool enable_comp = schedulers[0]->enable_comp_;

  int scheme = 1;
  // NVS allocation
  if (scheme == 0) {
    for (int rb_id = 0; rb_id < nb_of_rbs; rb_id++) {
      NVSAllocateOneRB(schedulers, rb_id, enable_comp);
    }
  }
  else if (scheme == 1) {
    for (auto it = schedulers.begin(); it != schedulers.end(); it++) {
      RadioSaberDownlinkScheduler* scheduler = (RadioSaberDownlinkScheduler*)(*it);
      scheduler->CalculateSliceQuota();
    }
    for (int rb_id = 0; rb_id < nb_of_rbs; rb_id++) {
      RadioSaberAllocateOneRB(schedulers, rb_id, enable_comp);
    }
  }
  else if (scheme == 2) {
    // Baseline3 allocation
    SliceContext& slice_ctx = schedulers[0]->slice_ctx_;
    std::vector<int> slice_quota(slice_ctx.num_slices_, 0);
    if (slice_offset_.size() == 0)
      slice_offset_.resize(slice_quota.size(), 0);
    std::cout << "global radiosaber: ";
    for (int i = 0; i < slice_quota.size(); i++) {
      slice_quota[i] = nb_of_rbs * schedulers.size() * slice_ctx.weights_[i]
        + slice_offset_[i];
      std::cout << "(" << slice_quota[i] << ", " << slice_offset_[i] << ") ";
    }
    std::cout << std::endl;
    for (int rb_id = 0; rb_id < nb_of_rbs; rb_id++) {
      RadioSaberAllocateOneRBGlobal(
        schedulers, rb_id, enable_comp,slice_quota);
    }
    slice_offset_ = slice_quota;
  }

  for (int j = 0; j < schedulers.size(); j++) {
    schedulers[j]->FinalizeScheduledFlows();
    schedulers[j]->StopSchedule();
  }
}

void
FrameManager::NVSAllocateOneRB(
    std::vector<DownlinkPacketScheduler*>& schedulers,
    int rb_id, bool enable_comp) {
  AMCModule* amc = schedulers[0]->GetMacEntity()->GetAmcModule();
  // metrics[j][k] is the scheduling metric of the k-th flow in j-th cell
  std::vector<std::vector<double>> metrics;
  for (int j = 0; j < schedulers.size(); j++) {
    metrics.emplace_back();
    FlowsToSchedule* flows = schedulers[j]->GetFlowsToSchedule();
    for (int k = 0; k < flows->size(); k++) {
      double metric = schedulers[j]->ComputeSchedulingMetric(
          flows->at(k)->GetBearer(),
          flows->at(k)->GetSpectralEfficiency().at(rb_id),
          rb_id);
      metrics[j].push_back(metric);
    }
  }
  std::unordered_set<int> cells_allocated;
  std::vector<FlowToSchedule*> cell_flows(schedulers.size(), nullptr);
  std::vector<int> cell_byorder;
  while (true) {
    // allocate rb_id-th rb to a specific flow in a specific cell in greedy
    // we consider the limited queue of every flow in the future
    int schedule_cell_id = -1;
    double target_metric = -1;
    FlowToSchedule* schedule_flow = nullptr;
    for (int j = 0; j < schedulers.size(); j++) {
      if (cells_allocated.find(j) != cells_allocated.end()) {
        continue;
      }
      FlowsToSchedule* flows = schedulers[j]->GetFlowsToSchedule();
      for (int k = 0; k < flows->size(); k++) {
        if (metrics[j][k] >= target_metric) { 
          target_metric = metrics[j][k];
          schedule_cell_id = j;
          schedule_flow = flows->at(k);
        }
      }
    }
    if (schedule_cell_id == -1) {
      // all cells are allocated
      break;
    }
    cells_allocated.insert(schedule_cell_id);
    cell_flows[schedule_cell_id] = schedule_flow;
    cell_byorder.push_back(schedule_cell_id);
  }
  // compare the TBS and decide muting
  cells_allocated.clear();
  std::unordered_set<int> cells_muted;
  for (int j = 0; j < cell_byorder.size(); j++) {
    int cell_id = cell_byorder[j];
    if (cells_muted.find(cell_id) != cells_muted.end()) {
      continue; // the cell is muted, skip
    }
    FlowToSchedule* flow = cell_flows[cell_id];
    CqiReport& cqi_report = flow->GetCqiWithMuteFeedbacks().at(rb_id);
    cqi_report.final_cqi = cqi_report.cqi;
    flow->GetListOfAllocatedRBs()->push_back(rb_id);
    cells_allocated.insert(cell_id);
    // no neighbor cell or comp disabled, skip
    if (cqi_report.neighbor_cell == -1 || !enable_comp) {
      continue;
    }
    // Muting logic
    int tbs_with_mute = amc->GetTBSizeFromMCS(
        amc->GetMCSFromCQI(cqi_report.cqi_with_mute));
    int tbs = amc->GetTBSizeFromMCS(amc->GetMCSFromCQI(cqi_report.cqi));
    FlowToSchedule* another_flow = cell_flows[cqi_report.neighbor_cell];
    CqiReport another_report = another_flow->GetCqiWithMuteFeedbacks().at(rb_id);
    int tbs_another = amc->GetTBSizeFromMCS(amc->GetMCSFromCQI(another_report.cqi));

    // if the neighbor cell is not allocated
    if (cells_allocated.find(cqi_report.neighbor_cell) == cells_allocated.end()) {
      if (cells_muted.find(cqi_report.neighbor_cell) != cells_muted.end()) {
        cqi_report.final_cqi = cqi_report.cqi_with_mute;
      }
      else if (tbs_with_mute > 1.0 * (tbs_another + tbs)) {
#ifdef SCHEDULER_DEBUG
        std::cout << "Mute cell " << cqi_report.neighbor_cell
          << " rb " << rb_id << " for flow "
          << flow->GetBearer()->GetApplication()->GetApplicationID()
          << " tbs_with_mute: " << tbs_with_mute
          << " original_tbs: " << tbs
          << " another_tbs: " << tbs_another << std::endl;
#endif
        cells_muted.insert(cqi_report.neighbor_cell);
        cqi_report.final_cqi = cqi_report.cqi_with_mute;
      }
    }
  }
}

void
FrameManager::RadioSaberAllocateOneRB(
    std::vector<DownlinkPacketScheduler*>& schedulers,
    int rb_id, bool enable_comp) {
  AMCModule* amc = schedulers[0]->GetMacEntity()->GetAmcModule();
  for (auto it_s = schedulers.begin(); it_s != schedulers.end(); it_s++) {
    RadioSaberDownlinkScheduler* scheduler = (RadioSaberDownlinkScheduler*)(*it_s);
    FlowsToSchedule* flows = scheduler->GetFlowsToSchedule();
    int num_slice = scheduler->slice_ctx_.num_slices_;
    std::vector<FlowToSchedule*> slice_flow(num_slice, nullptr);
    std::vector<double> slice_spectraleff(num_slice, -1);
    std::vector<int> max_metrics(num_slice, -1);
    // calcualte the metrics and get the scheduled flow in every slice
    for (auto it = flows->begin(); it != flows->end(); it++) {
      FlowToSchedule* flow = *it;
      double metric = scheduler->ComputeSchedulingMetric(
          flow->GetBearer(),
          flow->GetSpectralEfficiency().at(rb_id),
          rb_id);
      int slice_id = flow->GetSliceID();
      // enterprise schedulers
      if (metric > max_metrics[slice_id]) {
        max_metrics[slice_id] = metric;
        slice_flow[slice_id] = flow;
        slice_spectraleff[slice_id] = flow->GetSpectralEfficiency().at(rb_id);
      }
    }
    double max_slice_spectraleff = -1;
    FlowToSchedule* selected_flow = nullptr;
    for (int i = 0; i < num_slice; i++) {
      if (slice_spectraleff[i] > max_slice_spectraleff
        && scheduler->slice_target_rbs_[i] > 0) {
        max_slice_spectraleff = slice_spectraleff[i];
        selected_flow = slice_flow[i];
      }
    }
    if (selected_flow) {
      selected_flow->GetListOfAllocatedRBs()->push_back(rb_id);
      scheduler->slice_target_rbs_[selected_flow->GetSliceID()] -= 1;
    }
  }
  // after allocation of one RB, reduce the slice_target_rbs_ by one;
}

void
FrameManager::RadioSaberAllocateOneRBGlobal(
    std::vector<DownlinkPacketScheduler*>& schedulers,
    int rb_id, bool enable_comp,
    std::vector<int>& slice_quota)
{
  AMCModule* amc = schedulers[0]->GetMacEntity()->GetAmcModule();
  for (auto it_s = schedulers.begin(); it_s != schedulers.end(); it_s++) {
    RadioSaberDownlinkScheduler* scheduler = (RadioSaberDownlinkScheduler*)(*it_s);
    FlowsToSchedule* flows = scheduler->GetFlowsToSchedule();
    int num_slice = scheduler->slice_ctx_.num_slices_;
    std::vector<FlowToSchedule*> slice_flow(num_slice, nullptr);
    std::vector<double> slice_spectraleff(num_slice, -1);
    std::vector<int> max_metrics(num_slice, -1);
    std::unordered_set<int> slice_with_flow;
    // calcualte the metrics and get the scheduled flow in every slice
    for (auto it = flows->begin(); it != flows->end(); it++) {
      FlowToSchedule* flow = *it;
      double metric = scheduler->ComputeSchedulingMetric(
          flow->GetBearer(),
          flow->GetSpectralEfficiency().at(rb_id),
          rb_id);
      int slice_id = flow->GetSliceID();
      // enterprise schedulers
      if (metric > max_metrics[slice_id]) {
        max_metrics[slice_id] = metric;
        slice_flow[slice_id] = flow;
        slice_spectraleff[slice_id] = flow->GetSpectralEfficiency().at(rb_id);
        slice_with_flow.insert(slice_id);
      }
    }
    if (slice_with_flow.size() == 0) {
      continue;
    }
    double max_slice_spectraleff = -1;
    FlowToSchedule* selected_flow = nullptr;
    for (auto it = slice_with_flow.begin(); it != slice_with_flow.end(); it++) {
      int slice_id = *it;
      if (slice_spectraleff[slice_id] > max_slice_spectraleff
        && slice_quota[slice_id] > 0) {
        max_slice_spectraleff = slice_spectraleff[slice_id];
        selected_flow = slice_flow[slice_id];
      }
    }
    if (!selected_flow) {
      // if no slice has quota(it's possible due to uneven distribution of users in cells)
      int k = rand() % slice_with_flow.size();
      for (auto it = slice_with_flow.begin(); it != slice_with_flow.end(); it++) {
        if (k == 0) {
          selected_flow = slice_flow[(*it)];
          break;
        }
        k -= 1;
      }
    }
    if (selected_flow) {
      selected_flow->GetListOfAllocatedRBs()->push_back(rb_id);
      slice_quota[selected_flow->GetSliceID()] -= 1;
    }
  }
  // after allocation of one RB, reduce the slice_target_rbs_ by one;
}