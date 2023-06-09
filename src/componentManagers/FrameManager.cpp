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
#include "../device/UserEquipment.h"
#include "../protocolStack/mac/packet-scheduler/downlink-packet-scheduler.h"
#include "../protocolStack/mac/AMCModule.h"
#include "../core/spectrum/bandwidth-manager.h"
#include "../utility/eesm-effective-sinr.h"
#include "../flows/radio-bearer.h"
#include "../flows/application/Application.h"
#include "../phy/enb-lte-phy.h"
#include <cassert>
#include <unordered_set>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <random>
#include <stdexcept>
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
      << Simulator::Init()->Now() << " --------- " << "TTI: " << GetTTICounter() << std::endl;
#endif

  // Prints UE positioning logs after the handovers have occured.
  if(GetTTICounter() == 120){
    std::vector<ENodeB*> *enodebs = GetNetworkManager ()->GetENodeBContainer ();
    cout << "ENB Size: " << enodebs->size() << endl;
    for (auto iter = enodebs->begin (); iter != enodebs->end (); iter++) {
  	  ENodeB* enb = *iter;
      ENodeB::UserEquipmentRecords* ue_records = enb->GetUserEquipmentRecords();
      cout << "UE Records Size: " << ue_records->size() << endl;
      for (int i = 0; i < ue_records->size(); i++){
          UserEquipment* x = (*ue_records)[i]->GetUE();
          cout << GetTTICounter();
          x->Print();
      }
    }
  }

  // if(GetTTICounter() == 2990){
  //   std::vector<ENodeB*> *enodebs = GetNetworkManager ()->GetENodeBContainer ();
  //   for (auto iter = enodebs->begin (); iter != enodebs->end (); iter++) {
  // 	  ENodeB* enb = *iter;
  //     ENodeB::UserEquipmentRecords* ue_records = enb->GetUserEquipmentRecords();
  //     for (int i = 0; i < ue_records->size(); i++){
  //         UserEquipment* x = (*ue_records)[i]->GetUE();
  //         cout << GetTTICounter();
  //         x->Print();
  //     }
  //   }
  // }

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
  int counter = 0;
  for (auto it = enodebs->begin(); it != enodebs->end(); it++) {
    ENodeB* enb = *it;
    DownlinkPacketScheduler* scheduler =
      (DownlinkPacketScheduler*)enb->GetDLScheduler();
    assert(counter == enb->GetIDNetworkNode());
    schedulers.push_back(scheduler);
    counter += 1;
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
  // int nb_of_rbs = no_of_schedulers * 4;
  // cout << "Number of RBS: " << nb_of_rbs << endl;
  // bool enable_comp = schedulers[0]->enable_comp_;

  if (schedulers[0]->enable_tune_weights_) {
    TuneWeightsAcrossCells(schedulers);
    schedulers[0]->enable_tune_weights_ = false;
  }

  int scheme = 1;
  assert(scheme == 1);
  if (scheme == 0) {
    for (int rb_id = 0; rb_id < nb_of_rbs; rb_id++) {
      NVSAllocateOneRB(schedulers, rb_id);
    }
  }
  else if (scheme == 1) {
    for (auto it = schedulers.begin(); it != schedulers.end(); it++) {
      RadioSaberDownlinkScheduler* scheduler = (RadioSaberDownlinkScheduler*)(*it);
      scheduler->CalculateSliceQuota();
    }
    int rb_id = 0;
    while (rb_id < nb_of_rbs) { // Allocate in the form of PRBG of size 4
      RadioSaberAllocateOneRB(schedulers, rb_id);
      // RadioSaberAllocateOneRBSecondMute(schedulers, rb_id);
      rb_id += RBG_SIZE;
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
        schedulers, rb_id, slice_quota);
    }
    slice_offset_ = slice_quota;
  }

  for (int j = 0; j < schedulers.size(); j++) {
    schedulers[j]->FinalizeScheduledFlows();
    schedulers[j]->StopSchedule();
  }
}

inline int provision_metric(double weight, double ideal_weight) {
  double weight_diff = weight - ideal_weight;
  double delta = 0.01;
  if (abs(weight_diff) < delta) {
      return 0;
  }
  if (weight_diff >= delta) {
    return 1;
  }
  else {
    return -1;
  }
}

inline bool within_range(double v) {
  return v >= 0 && v <= 1;
}

bool
FrameManager::TuneWeightsAcrossCells(std::vector<DownlinkPacketScheduler*>& schedulers)
{
  std::vector<std::vector<int>> slice_users_per_cell;
  std::vector<std::vector<double>> ideal_weight_per_cell;
  auto& slice_weight = schedulers[0]->slice_ctx_.weights_;
  std::vector<int> slice_users(slice_weight.size(), 0);
  assert(slice_weight.size() == schedulers[0]->slice_ctx_.num_slices_);
  // calculate ideal weights of slices in every cell
  for (int j = 0; j < schedulers.size(); j++) {
    slice_users_per_cell.emplace_back(slice_weight.size(), 0);
    ideal_weight_per_cell.emplace_back(slice_weight.size(), 0);
    auto flows = schedulers[j]->GetFlowsToSchedule();
    for (auto it = flows->begin(); it != flows->end(); it++) {
      FlowToSchedule* flow = *it;
      int slice_id = flow->GetSliceID();
      slice_users_per_cell[j][slice_id] += 1;
      slice_users[slice_id] += 1;
    }
  }
  for (int j = 0; j < schedulers.size(); j++) {
    for (int i = 0; i < slice_weight.size(); i++) {
      ideal_weight_per_cell[j][i] = (double)slice_users_per_cell[j][i] / slice_users[i]
        * slice_weight[i] * schedulers.size();
    }
  }
  const double delta = 0.01;
  // 1 indicates over provision; -1 indicates under-provision
  std::vector<std::vector<int>> provision_matrix;
  for (int cell_id = 0; cell_id < schedulers.size(); cell_id++) {
    provision_matrix.emplace_back();
    for (int slice_id = 0; slice_id < slice_weight.size(); slice_id++) {
      double metric = provision_metric(
          schedulers[cell_id]->slice_ctx_.weights_[slice_id],
          ideal_weight_per_cell[cell_id][slice_id]);
      provision_matrix[cell_id].push_back(metric);
    }
  }
  
  /*
  // simulated annealing
  int counter = 0;
  double temp = 1;
  const double cooling_alpha = 0.99;
  std::random_device rd;
  std::mt19937 e2(rd());
  std::uniform_real_distribution<> distribution(0, 1);
  while (counter < 2000) {
    int lcell = rand() % schedulers.size();
    int rcell = rand() % schedulers.size();
    while (rcell == lcell) rcell = rand() % schedulers.size();
    int lslice = rand() % slice_weight.size();
    int rslice = rand() % slice_weight.size();
    while (rslice == lslice) rslice = rand() % slice_weight.size();
    if (provision_matrix[lcell][lslice] == 0)
      continue;
    int over = provision_matrix[lcell][lslice];
    double sum_diff = 0;
    for (int cell_id = 0; cell_id < schedulers.size(); cell_id++) {
      for (int slice_id = 0; slice_id < slice_weight.size(); slice_id++) {
        sum_diff += pow(ideal_weight_per_cell[cell_id][slice_id] - 
          schedulers[cell_id]->slice_ctx_.weights_[slice_id], 2);
      }
    }
    double lc_ls = schedulers[lcell]->slice_ctx_.weights_[lslice];
    double lc_rs = schedulers[lcell]->slice_ctx_.weights_[rslice];
    double rc_ls = schedulers[rcell]->slice_ctx_.weights_[lslice];
    double rc_rs = schedulers[rcell]->slice_ctx_.weights_[rslice];
    double origin_loss = 
        pow(lc_ls - ideal_weight_per_cell[lcell][lslice], 2)
      + pow(lc_rs - ideal_weight_per_cell[lcell][rslice], 2)
      + pow(rc_ls - ideal_weight_per_cell[rcell][lslice], 2)
      + pow(rc_rs - ideal_weight_per_cell[rcell][rslice], 2);
    double new_loss = 
        pow(lc_ls - ideal_weight_per_cell[lcell][lslice] - over * delta, 2)
      + pow(lc_rs - ideal_weight_per_cell[lcell][rslice] + over * delta, 2)
      + pow(rc_ls - ideal_weight_per_cell[rcell][lslice] + over * delta, 2)
      + pow(rc_rs - ideal_weight_per_cell[rcell][rslice] - over * delta, 2);
    bool inrange = within_range(lc_ls - over*delta) 
      && within_range(lc_rs + over * delta) && within_range(rc_ls + over * delta)
      && within_range(rc_rs - over * delta);
    double delta_loss = new_loss - origin_loss;
    std::cout << counter << " temp: " << temp << " delta: " << delta_loss
      << " total_loss: " << sum_diff << std::endl;
    if ( inrange && (delta_loss < 0 || distribution(e2) < exp(-delta_loss / temp)) ) {
      std::vector<int> cells;
      cells.push_back(lcell);
      cells.push_back(rcell);
      std::vector<int> slices;
      slices.push_back(lslice);
      slices.push_back(rslice);
      for (auto it = cells.begin(); it != cells.end(); it++) {
        for (auto it_s = slices.begin(); it_s != slices.end(); it_s++) {
          int cell_id = *it;
          int slice_id = *it_s;
          std::cout << "(" << schedulers[cell_id]->slice_ctx_.weights_[slice_id]
            << ", " << ideal_weight_per_cell[cell_id][slice_id]
            << ")" << "; ";  
        }
      }
      std::cout << std::endl;
      schedulers[lcell]->slice_ctx_.weights_[lslice] -= over * delta;
      schedulers[lcell]->slice_ctx_.weights_[rslice] += over * delta;
      schedulers[rcell]->slice_ctx_.weights_[lslice] += over * delta;
      schedulers[rcell]->slice_ctx_.weights_[rslice] -= over * delta;
      provision_matrix[lcell][lslice] = provision_metric(
        schedulers[lcell]->slice_ctx_.weights_[lslice], ideal_weight_per_cell[lcell][lslice]);
      provision_matrix[lcell][rslice] = provision_metric(
        schedulers[lcell]->slice_ctx_.weights_[rslice], ideal_weight_per_cell[lcell][rslice]);
      provision_matrix[rcell][lslice] = provision_metric(
        schedulers[rcell]->slice_ctx_.weights_[lslice], ideal_weight_per_cell[rcell][lslice]);
      provision_matrix[rcell][rslice] = provision_metric(
        schedulers[rcell]->slice_ctx_.weights_[rslice], ideal_weight_per_cell[rcell][rslice]);

      for (auto it = cells.begin(); it != cells.end(); it++) {
        for (auto it_s = slices.begin(); it_s != slices.end(); it_s++) {
          int cell_id = *it;
          int slice_id = *it_s;
          std::cout << "(" << schedulers[cell_id]->slice_ctx_.weights_[slice_id]
            << ", " << ideal_weight_per_cell[cell_id][slice_id]
            << ")" << "; ";  
        }
      }
      std::cout << std::endl;
    }
    temp *= cooling_alpha;
    counter += 1;
  }
  double sum_diff = 0;
  for (int cell_id = 0; cell_id < schedulers.size(); cell_id++) {
    std::cout << "cell " << cell_id << ": ";
    for (int slice_id = 0; slice_id < slice_weight.size(); slice_id++) {
      std::cout << "(" << schedulers[cell_id]->slice_ctx_.weights_[slice_id]
        << ", " << provision_matrix[cell_id][slice_id]
        << ", " << ideal_weight_per_cell[cell_id][slice_id]
        << ")" << "; ";
      sum_diff += pow(ideal_weight_per_cell[cell_id][slice_id] - 
        schedulers[cell_id]->slice_ctx_.weights_[slice_id], 2);
    }
    std::cout << std::endl;
  }
  std::cout << "total_loss: " << sum_diff << std::endl;
  */
  // initial greedy algorithm: we keep reallocating when there's a chance
  int counter = 0;
  while (true) {
    bool swap_once = false;
    std::cout << "swap_iteration: " << counter << std::endl;
    counter += 1;
    for (int lcell = 0; lcell < schedulers.size(); lcell++) {
      for (int lslice = 0; lslice < slice_weight.size(); lslice++) {
        for (int rslice = 0; rslice < slice_weight.size(); rslice++) {
          if (lslice == rslice) continue;
          for (int rcell = 0; rcell < schedulers.size(); rcell++) {
            if (lcell == rcell) continue;
            if (provision_matrix[lcell][lslice] * provision_matrix[lcell][rslice] == -1) {
              // both provision or under-provision for both slices across two cells, skip
              if (provision_matrix[lcell][lslice] * provision_matrix[rcell][lslice] == 1
                && provision_matrix[lcell][rslice] * provision_matrix[rcell][rslice] == 1)
                continue;
              // if any 0 is achieved in rcell, skip
              if (provision_matrix[rcell][lslice] * provision_matrix[rcell][rslice] == 0)
                continue;
              int over = provision_matrix[lcell][lslice];
              schedulers[lcell]->slice_ctx_.weights_[lslice] -= over * delta;
              schedulers[rcell]->slice_ctx_.weights_[lslice] += over * delta;
              schedulers[lcell]->slice_ctx_.weights_[rslice] += over * delta;
              schedulers[rcell]->slice_ctx_.weights_[rslice] -= over * delta;
              swap_once = true;
              provision_matrix[lcell][lslice] = provision_metric(
                schedulers[lcell]->slice_ctx_.weights_[lslice],
                ideal_weight_per_cell[lcell][lslice]);
              provision_matrix[lcell][rslice] = provision_metric(
                schedulers[lcell]->slice_ctx_.weights_[rslice],
                ideal_weight_per_cell[lcell][rslice]);
              provision_matrix[rcell][lslice] = provision_metric(
                schedulers[rcell]->slice_ctx_.weights_[lslice],
                ideal_weight_per_cell[rcell][lslice]);
              provision_matrix[rcell][rslice] = provision_metric(
                schedulers[rcell]->slice_ctx_.weights_[rslice],
                ideal_weight_per_cell[rcell][rslice]);
            }
          }
        }
      }
    }
    if (!swap_once) break;
  }
  double sum_diff = 0;
  for (int cell_id = 0; cell_id < schedulers.size(); cell_id++) {
    std::cout << "cell " << cell_id << ": ";
    for (int slice_id = 0; slice_id < slice_weight.size(); slice_id++) {
      std::cout << "(" << schedulers[cell_id]->slice_ctx_.weights_[slice_id]
        << ", " << provision_matrix[cell_id][slice_id]
        << ", " << ideal_weight_per_cell[cell_id][slice_id]
        << ")" << "; ";
      sum_diff += pow(ideal_weight_per_cell[cell_id][slice_id] - 
        schedulers[cell_id]->slice_ctx_.weights_[slice_id], 2);
    }
    std::cout << std::endl;
  }
  std::cout << "total_loss: " << sum_diff << std::endl;
}

void
FrameManager::NVSAllocateOneRB(
    std::vector<DownlinkPacketScheduler*>& schedulers,
    int rb_id) {
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
    // currently assume every flow is backlogged
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
  FinalizeAllocation(cell_byorder, schedulers, cell_flows, rb_id);
}

void
FrameManager::RadioSaberAllocateOneRBSecondMute(
    std::vector<DownlinkPacketScheduler*>& schedulers,
    int rb_id) {
  AMCModule* amc = schedulers[0]->GetMacEntity()->GetAmcModule();
  std::unordered_set<int> global_cells_muted;
  int global_sum_tbs = 0;
  std::vector<FlowToSchedule*> global_cell_flow;
  const int index_no_mute = schedulers.size();
  // std::cout << "muting iteration for rb: " << rb_id << std::endl;
  while (true) {
    int final_mute_id = -1;
    int max_sum_tbs = 0;
    std::vector<FlowToSchedule*> max_cell_flow;
    // if not enable_comp, start from schedulers.size() which means no muting
    // if enable_comp, start from every possible muting cell
    int mute_id = schedulers[0]->enable_comp_ ? 0 : index_no_mute;
    for (; mute_id <= schedulers.size(); mute_id++) {
      // already muted
      if (global_cells_muted.count(mute_id)) {
        continue;
      }
      int sum_tbs = 0;
      std::vector<FlowToSchedule*> cell_flow(schedulers.size(), nullptr);
      for (int j = 0; j < schedulers.size(); j++) {
        RadioSaberDownlinkScheduler* scheduler =
          (RadioSaberDownlinkScheduler*)schedulers[j];
        // skip the muted cell
        if (j == mute_id || global_cells_muted.count(j)) {
          continue;
        }
        FlowsToSchedule* flows = scheduler->GetFlowsToSchedule();
        int num_slice = scheduler->slice_ctx_.num_slices_;
        // enterprise scheduling for every slice
        std::vector<FlowToSchedule*> slice_flow(num_slice, nullptr);
        std::vector<double> slice_spectraleff(num_slice, -1);
        std::vector<int> max_metrics(num_slice, -1);
        for (auto it = flows->begin(); it != flows->end(); it++) {
          FlowToSchedule* flow = *it;
          auto& rsrp_report = flow->GetRSRPReport();
          // under the current muting assumption
          double spectraleff_rbg = 0.0;
          for (int i = rb_id; i < RBG_SIZE+rb_id; i++) {
            double in_under_mute = rsrp_report.noise_interfere_watt[i];
            if (mute_id != index_no_mute) {
              in_under_mute -= pow(10., rsrp_report.rsrp_interference[mute_id]/10.);
            }
            int cqi_under_mute = amc->GetCQIFromSinr(
                rsrp_report.rx_power[i] - 10. * log10(in_under_mute));
            spectraleff_rbg += amc->GetEfficiencyFromCQI(cqi_under_mute);
          }
          double metric = scheduler->ComputeSchedulingMetric(
              flow->GetBearer(), spectraleff_rbg, rb_id);
          int slice_id = flow->GetSliceID();
          // enterprise schedulers
          if (metric > max_metrics[slice_id]) {
            max_metrics[slice_id] = metric;
            slice_flow[slice_id] = flow;
            slice_spectraleff[slice_id] = spectraleff_rbg;
          }
        }
        double max_slice_spectraleff = -1;
        FlowToSchedule* selected_flow = nullptr;
        for (int i = 0; i < num_slice; i++) {
          if (slice_spectraleff[i] > max_slice_spectraleff
              && scheduler->slice_rbgs_quota_[i] > 0) {
            max_slice_spectraleff = slice_spectraleff[i];
            selected_flow = slice_flow[i];
          }
        }
        if (selected_flow) {
          cell_flow[j] = selected_flow;
          sum_tbs += max_slice_spectraleff;
        }
      }
      if (sum_tbs > max_sum_tbs) {
        max_sum_tbs = sum_tbs;
        final_mute_id = mute_id;
        max_cell_flow = cell_flow;
      }
    }
    if (final_mute_id == index_no_mute) {
      global_sum_tbs = max_sum_tbs;
      global_cell_flow = max_cell_flow;
      // std::cout << "( null, " << global_sum_tbs << ") " << std::endl;
      // terminate the loop since no more benefit by muting a cell
      break;
    }
    else {
      global_sum_tbs = max_sum_tbs;
      global_cells_muted.insert(final_mute_id);
      // std::cout << "Muting Cell: (" << final_mute_id << ", " << global_sum_tbs << ") ";
      global_cell_flow = max_cell_flow;
      // update all flows of cell j
      for (int j = 0; j < schedulers.size(); j++) {
        if (j != final_mute_id) {
          FlowsToSchedule* flows = schedulers[j]->GetFlowsToSchedule();
          for (auto it = flows->begin(); it != flows->end(); it++) {
            auto& rsrp_report = (*it)->GetRSRPReport();
            for (int i = rb_id; i < RBG_SIZE+rb_id; i++) {
              rsrp_report.noise_interfere_watt[i] -= pow(10., rsrp_report.rsrp_interference[final_mute_id]/10.);
            }
          }
        }
      }
    }
  }
  for (int j = 0; j < schedulers.size(); j++) {
    if (global_cells_muted.count(j)) {
      continue;
    }
    RadioSaberDownlinkScheduler* scheduler =
      (RadioSaberDownlinkScheduler*)schedulers[j];
    FlowToSchedule* flow = global_cell_flow[j];
    auto& rsrp_report = flow->GetRSRPReport();
    // under the current muting assumption
    for (int i = rb_id; i < RBG_SIZE+rb_id; i++) {
      double in_under_mute = rsrp_report.noise_interfere_watt[i];
      int cqi_under_mute = amc->GetCQIFromSinr(
        rsrp_report.rx_power[i] - 10. * log10(in_under_mute));
      flow->GetCqiFeedbacks()[i] = cqi_under_mute;
    }
    scheduler->slice_rbgs_quota_[flow->GetSliceID()] -= 1;
    for (int i = rb_id; i < RBG_SIZE+rb_id; i++){
      flow->GetListOfAllocatedRBs()->push_back(i);
    }
  }
}

// void
// FrameManager::RadioSaberAllocateOneRBSecondMute(
//     std::vector<DownlinkPacketScheduler*>& schedulers,
//     int rb_id) {
//   AMCModule* amc = schedulers[0]->GetMacEntity()->GetAmcModule();
//   int max_sum_tbs = 0;
//   int final_mute_id = schedulers.size();  // which means no cell is muted
//   std::vector<std::vector<FlowToSchedule*>> cell_flows_with_mute(schedulers.size());
//   std::vector<int> tbs_with_mute;
//   for (int mute_id = 0; mute_id <= schedulers.size(); mute_id++) {
//     int sum_tbs = 0;
//     for (int j = 0; j < schedulers.size(); j++) {
//       RadioSaberDownlinkScheduler* scheduler = (RadioSaberDownlinkScheduler*)schedulers[j];
//       // skip the muted cell
//       if (j == mute_id) {
//         cell_flows_with_mute[j].push_back(nullptr);
//         continue;
//       }
//       FlowsToSchedule* flows = scheduler->GetFlowsToSchedule();
//       int num_slice = scheduler->slice_ctx_.num_slices_;
//       // enterprise scheduling for every slice
//       std::vector<FlowToSchedule*> slice_flow(num_slice, nullptr);
//       std::vector<double> slice_spectraleff(num_slice, -1);
//       std::vector<int> max_metrics(num_slice, -1);
//       for (auto it = flows->begin(); it != flows->end(); it++) {
//         FlowToSchedule* flow = *it;
//         auto& cqi_report = flow->GetCqiWithMuteFeedbacks();
//         // under the current muting assumption
//         double spectraleff_rbg = 0.0;
//         for (int i = 0; i < RBG_SIZE; i++) {
//           int cqi_under_mute = cqi_report[rb_id+i].cqi;
//           if (mute_id == cqi_report[rb_id+i].cell_one) {
//             cqi_under_mute = cqi_report[rb_id+i].cqi_mute_one;
//           }
//           else if (mute_id == cqi_report[rb_id+i].cell_two) {
//             cqi_under_mute = cqi_report[rb_id+i].cqi_mute_two;
//           }
//           spectraleff_rbg += amc->GetEfficiencyFromCQI(cqi_under_mute);
//         }
//         double metric = scheduler->ComputeSchedulingMetric(flow->GetBearer(),
//           spectraleff_rbg, rb_id);
//         int slice_id = flow->GetSliceID();
//         // enterprise schedulers
//         if (metric > max_metrics[slice_id]) {
//           max_metrics[slice_id] = metric;
//           slice_flow[slice_id] = flow;
//           slice_spectraleff[slice_id] = spectraleff_rbg;
//         }
//       }
//       double max_slice_spectraleff = -1;
//       FlowToSchedule* selected_flow = nullptr;
//       for (int i = 0; i < num_slice; i++) {
//         if (slice_spectraleff[i] > max_slice_spectraleff
//           && scheduler->slice_rbgs_quota_[i] > 0) {
//           max_slice_spectraleff = slice_spectraleff[i];
//           selected_flow = slice_flow[i];
//         }
//       }
//       if (selected_flow) {
//         cell_flows_with_mute[j].push_back(selected_flow);
//         sum_tbs += max_slice_spectraleff;
//       }
//     }
//     tbs_with_mute.push_back(sum_tbs);
//     if (sum_tbs > max_sum_tbs) {
//       max_sum_tbs = sum_tbs;
//       final_mute_id = mute_id;
//     }
//   }
//   std::cout << "RB: " << rb_id << " tbs with mutes: ";
//   for (int i = 0; i < tbs_with_mute.size(); i++) {
//     std::cout << tbs_with_mute[i] << ", ";
//   }
//   std::cout << std::endl;
//   for (int j = 0; j < schedulers.size(); j++) {
//     if (j == final_mute_id) {
//       continue;
//     }
//     RadioSaberDownlinkScheduler* scheduler = (RadioSaberDownlinkScheduler*)schedulers[j];
//     FlowToSchedule* flow = cell_flows_with_mute[j][final_mute_id];
//     auto& cqi_report = flow->GetCqiWithMuteFeedbacks();
//     // under the current muting assumption
//     for (int i = 0; i < RBG_SIZE; i++) {
//       if (final_mute_id == cqi_report[rb_id+i].cell_one) {
//         cqi_report[rb_id+i].final_cqi = cqi_report[rb_id+i].cqi_mute_one;
//       }
//       else if (final_mute_id == cqi_report[rb_id+i].cell_two) {
//         cqi_report[rb_id+i].final_cqi = cqi_report[rb_id+i].cqi_mute_two;
//       }
//     }
//     scheduler->slice_rbgs_quota_[flow->GetSliceID()] -= 1;
//     for (int i = 0; i < RBG_SIZE; i++){
//       flow->GetListOfAllocatedRBs()->push_back(rb_id + i);
//     }
//   }
// }

void
FrameManager::RadioSaberAllocateOneRB(
    std::vector<DownlinkPacketScheduler*>& schedulers,
    int rb_id) {
  AMCModule* amc = schedulers[0]->GetMacEntity()->GetAmcModule();
  std::vector<FlowToSchedule*> cell_flows(schedulers.size(), nullptr);
  std::vector<std::pair<int, int>> cell_spectraleff(schedulers.size(), {0,0});
  for (int j = 0; j < schedulers.size(); j++) {
    RadioSaberDownlinkScheduler* scheduler = (RadioSaberDownlinkScheduler*)schedulers[j];
    FlowsToSchedule* flows = scheduler->GetFlowsToSchedule();
    int num_slice = scheduler->slice_ctx_.num_slices_;
    std::vector<FlowToSchedule*> slice_flow(num_slice, nullptr);
    std::vector<double> slice_spectraleff(num_slice, -1);
    std::vector<int> max_metrics(num_slice, -1);
    // calcualte the metrics and get the scheduled flow in every slice

    for (auto it = flows->begin(); it != flows->end(); it++) {
      FlowToSchedule* flow = *it;
      // Get Spectral Efficiency of the RBG
      double spectraleff_rbg = 0.0;
      for (int i = 0; i < RBG_SIZE; i++) {
        int cqi = flow->GetCqiWithMuteFeedbacks().at(rb_id+i).cqi;
        flow->GetCqiFeedbacks().at(rb_id+i) = cqi;
        spectraleff_rbg += amc->GetEfficiencyFromCQI(cqi);
      }
      double metric = scheduler->ComputeSchedulingMetric(
        flow->GetBearer(), spectraleff_rbg, rb_id);
      int slice_id = flow->GetSliceID();
      // enterprise schedulers
      if (metric > max_metrics[slice_id]) {
        max_metrics[slice_id] = metric;
        slice_flow[slice_id] = flow;
        slice_spectraleff[slice_id] = spectraleff_rbg;
      }
    }
    double max_slice_spectraleff = -1;
    FlowToSchedule* selected_flow = nullptr;
    for (int i = 0; i < num_slice; i++) {
      if (slice_spectraleff[i] > max_slice_spectraleff
        && scheduler->slice_rbgs_quota_[i] > 0) {
        max_slice_spectraleff = slice_spectraleff[i];
        selected_flow = slice_flow[i];
      }
    }
    if (selected_flow) {
      cell_flows[j] = selected_flow;
      cell_spectraleff[j].first = j;
      cell_spectraleff[j].second = max_slice_spectraleff;
      scheduler->slice_rbgs_quota_[selected_flow->GetSliceID()] -= 1;
    }
  }
  // after allocation of one RB, reduce the slice_rbgs_quota_ by one;
  sort(cell_spectraleff.begin(), cell_spectraleff.end(),
    [](const auto& a, const auto& b) -> bool {
      return a.second > b.second;
    });
  std::vector<int> cell_byorder;
  for (auto it = cell_spectraleff.begin(); it != cell_spectraleff.end(); it++) {
    cell_byorder.push_back(it->first);
  }
  // auto rng = std::default_random_engine();
  // std::shuffle(cell_byorder.begin(), cell_byorder.end(), rng);
  FinalizeAllocation(cell_byorder, schedulers, cell_flows, rb_id);
}

void
FrameManager::FinalizeAllocation(
  std::vector<int>& cell_byorder,
  std::vector<DownlinkPacketScheduler*>& schedulers,
  std::vector<FlowToSchedule*>& cell_flows, int rb_id)
{
  std::unordered_set<int> cells_allocated;
  std::unordered_set<int> cells_muted;
  bool enable_comp = schedulers[0]->enable_comp_;
  AMCModule* amc = schedulers[0]->GetMacEntity()->GetAmcModule();

  for (int j = 0; j < cell_byorder.size(); j++) {
    int cell_id = cell_byorder[j];
    if (cells_muted.find(cell_id) != cells_muted.end()) {
      continue; // the cell is muted, skip
    }
    FlowToSchedule* flow = cell_flows[cell_id];
    if (flow == nullptr) {
      throw std::runtime_error("cell with no flow");
    }
    CqiReport& cqi_report_first = flow->GetCqiWithMuteFeedbacks().at(rb_id);
    for (int i = 0; i < RBG_SIZE; i++){
      // Start i from i = 1 when you want to reserve 25% of the blocks for IM
      flow->GetListOfAllocatedRBs()->push_back(rb_id + i);
    }

    cells_allocated.insert(cell_id);
    // no neighbor cell or comp disabled, skip
    if (cqi_report_first.cell_one == -1 || !enable_comp) {
      continue;
    }
    // Muting logic
    int tbs_with_mute = 0, tbs = 0, tbs_neighbor = 0;
    int cell_one = cqi_report_first.cell_one;
    for (int i = 0; i < RBG_SIZE; i++) {
      CqiReport& cqi_report = flow->GetCqiWithMuteFeedbacks().at(rb_id + i);
      FlowToSchedule* neighbor_flow = cell_flows[cell_one];
      CqiReport neighbor_report = neighbor_flow->GetCqiWithMuteFeedbacks().at(rb_id);
      tbs_with_mute += amc->GetTBSizeFromMCS(
        amc->GetMCSFromCQI(cqi_report.cqi_mute_one));
      tbs += amc->GetTBSizeFromMCS(
        amc->GetMCSFromCQI(cqi_report.cqi));
      tbs_neighbor += amc->GetTBSizeFromMCS(
        amc->GetMCSFromCQI(neighbor_report.cqi));
    }

    // if the neighbor cell is not allocated
    if (cells_allocated.find(cell_one) == cells_allocated.end()) {
      if (cells_muted.find(cell_one) != cells_muted.end()) {
        for (int i = 0; i < RBG_SIZE; i++) {
          CqiReport& cqi_report = flow->GetCqiWithMuteFeedbacks().at(rb_id + i);
          flow->GetCqiFeedbacks()[rb_id+i] = cqi_report.cqi_mute_one;
        }
      }
      else if (tbs_with_mute > 1.5 * (tbs_neighbor + tbs)) {
#ifdef SCHEDULER_DEBUG
        std::cout << "Mute cell " << cell_one
          << " rbg " << rb_id / RBG_SIZE << " for flow "
          << flow->GetBearer()->GetApplication()->GetApplicationID()
          << " tbs_with_mute: " << tbs_with_mute
          << " original_tbs: " << tbs
          << " neighbor_tbs: " << tbs_neighbor << std::endl;
#endif
        cells_muted.insert(cell_one);
        for (int i = 0; i < RBG_SIZE; i++) {
          CqiReport& cqi_report = flow->GetCqiWithMuteFeedbacks().at(rb_id + i);
          flow->GetCqiFeedbacks()[rb_id+i] = cqi_report.cqi_mute_one;
        }
        if ((typeid(schedulers[cell_id]).name()) == "RadioSaberDownlinkScheduler") {
          RadioSaberDownlinkScheduler* neighbor = (RadioSaberDownlinkScheduler*)schedulers[cell_one];
          // TBC: there's some issues with this approach
          neighbor->slice_rbs_offset_[flow->GetSliceID()] -= 1;
        }
      }
    }
  }
}

void
FrameManager::RadioSaberAllocateOneRBGlobal(
    std::vector<DownlinkPacketScheduler*>& schedulers,
    int rb_id, std::vector<int>& slice_quota)
{
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
  // after allocation of one RB, reduce the slice_rbgs_quota_ by one;
}
