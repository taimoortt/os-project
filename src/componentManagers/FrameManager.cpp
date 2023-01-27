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
#include "../protocolStack/mac/packet-scheduler/packet-scheduler.h"
#include "../protocolStack/mac/AMCModule.h"
#include "../core/spectrum/bandwidth-manager.h"
#include "../utility/eesm-effective-sinr.h"
#include "../flows/radio-bearer.h"
#include "../flows/application/Application.h"
#include "../phy/enb-lte-phy.h"
#include <cassert>
#include <unordered_set>

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
  // Assume that every eNB has same number of RBs
  int nb_of_rbs = 0;
  int nb_of_cells = 0;
  int cells_with_flow = 0;
  // initialization
  for (auto it = enodebs->begin(); it != enodebs->end(); it++) {
    ENodeB* enb = *it;
    DownlinkPacketScheduler* scheduler =
      (DownlinkPacketScheduler*)enb->GetDLScheduler();
    assert(scheduler != NULL);
    int cell_rbs = scheduler->GetMacEntity()->GetDevice()->GetPhy()
      ->GetBandwidthManager()->GetDlSubChannels().size (); 
    if (nb_of_rbs == 0) {
      nb_of_rbs = cell_rbs;
    }
    else {
      assert(nb_of_rbs == cell_rbs);
    }
    scheduler->UpdateAverageTransmissionRate();
    scheduler->SelectFlowsToSchedule();
    schedulers.push_back(scheduler);
    if (scheduler->GetFlowsToSchedule()->size() != 0) {
      cells_with_flow += 1;
    }
  }
  nb_of_cells = schedulers.size();
  std::vector<std::unordered_set<int>> mute_rbs_cells;
  for (int i = 0; i < nb_of_rbs; i++) {
    mute_rbs_cells.emplace_back();
    // metrics[j][k] is the scheduling metric of the k-th flow in j-th cell
    std::vector<std::vector<double>> metrics;
    for (int j = 0; j < schedulers.size(); j++) {
      metrics.emplace_back();
      FlowsToSchedule* flows = schedulers[j]->GetFlowsToSchedule();
      for (int k = 0; k < flows->size(); k++) {
        double metric = schedulers[j]->ComputeSchedulingMetric(
            flows->at(k)->GetBearer(),
            flows->at(k)->GetSpectralEfficiency().at(i),
            i);
        metrics[j].push_back(metric);
      }
    }
    std::unordered_set<int> cells_muted;
    std::unordered_set<int> cells_allocated;
    while (true) {
      // allocate i-th rb to a specific flow in a specific cell in greedy
      // we consider the limited queue of every flow in the future
      int schedule_cell_id = -1;
      int schedule_flow_id = -1;
      double target_metric = -1;
      FlowToSchedule* schedule_flow = nullptr;
      for (int j = 0; j < schedulers.size(); j++) {
        if (cells_muted.find(j) != cells_muted.end() ||
            cells_allocated.find(j) != cells_allocated.end()) {
          continue;
        }
        FlowsToSchedule* flows = schedulers[j]->GetFlowsToSchedule();
        for (int k = 0; k < flows->size(); k++) {
          if (metrics[j][k] >= target_metric) { 
            target_metric = metrics[j][k];
            schedule_cell_id = j;
            schedule_flow_id = k;
            schedule_flow = flows->at(k);
          }
        }
      }
      if (schedule_cell_id == -1) {
        // all cells are allocated
        break;
      }
      schedule_flow->GetListOfAllocatedRBs()->push_back(i);
      std::vector<CqiReport>& cqi_with_mute =
        schedule_flow->GetCqiWithMuteFeedbacks();
      cells_allocated.insert(schedule_cell_id);
      int mute_cell_id = cqi_with_mute[i].neighbor_cell;

      /*enforce mute_cell_id = -1 to test*/
      mute_cell_id = -1;
      if (mute_cell_id != -1) {
        // applying RBs muting here
        if (cells_allocated.find(mute_cell_id) != cells_allocated.end()) {
          // cannot mute, apply the original cqi
          cqi_with_mute[i].final_cqi = cqi_with_mute[i].cqi;
        }
        else {
          if (cells_muted.find(mute_cell_id) == cells_muted.end()) {
            cells_muted.insert(mute_cell_id);
          }
          // apply the cqi_with_mute
          cqi_with_mute[i].final_cqi = cqi_with_mute[i].cqi_with_mute;
        }
      }
      else {
        // apply the original cqi
        cqi_with_mute[i].final_cqi = cqi_with_mute[i].cqi;
      }
    }
  }
  for (int j = 0; j < schedulers.size(); j++) {
#ifdef SCHEDULER_DEBUG
	  std::cout << " ---- RBsAllocation";
    std::cout << ", available RBs " << nb_of_rbs 
      << ", flows " << schedulers[j]->GetFlowsToSchedule()->size ()
      << std::endl;
#endif
    PdcchMapIdealControlMessage *pdcchMsg = new PdcchMapIdealControlMessage ();
    FlowsToSchedule* flows = schedulers[j]->GetFlowsToSchedule();
    AMCModule* amc = schedulers[j]->GetMacEntity()->GetAmcModule();
    for (auto it = flows->begin (); it != flows->end (); it++) {
      FlowToSchedule *flow = (*it);
      std::vector<int>* allocated_rbs = flow->GetListOfAllocatedRBs();
      if (allocated_rbs->size() > 0) {
        std::vector<double> sinr_values;
        for(auto it = allocated_rbs->begin(); it != allocated_rbs->end(); it++) {
          int rb_id = *it;
          double sinr = amc->GetSinrFromCQI(
              flow->GetCqiWithMuteFeedbacks().at(rb_id).final_cqi
              );
          sinr_values.push_back(sinr);
        }
        double effective_sinr = GetEesmEffectiveSinr(sinr_values);
        int mcs = amc->GetMCSFromCQI(amc->GetCQIFromSinr(effective_sinr));
        int tbs_size = amc->GetTBSizeFromMCS(mcs, allocated_rbs->size());
        flow->UpdateAllocatedBits(tbs_size);

#ifdef SCHEDULER_DEBUG
        std::cout << "\t\t --> flow "
          << flow->GetBearer()->GetApplication()->GetApplicationID()
          << " has been scheduled:"
          << " nb_of_RBs: " << flow->GetListOfAllocatedRBs ()->size ()
          << " effective_sinr: " << effective_sinr
          << " tbs_size: " << tbs_size
          << std::endl;
#endif
        for(auto it = allocated_rbs->begin(); it != allocated_rbs->end(); it++) {
          pdcchMsg->AddNewRecord(
              PdcchMapIdealControlMessage::DOWNLINK,
              *it, flow->GetBearer()->GetDestination(), mcs
              );
        }
      }
    }
    if (pdcchMsg->GetMessage()->size() > 0) {
      schedulers[j]->GetMacEntity()->GetDevice()
        ->GetPhy()->SendIdealControlMessage(pdcchMsg);
    }
    delete pdcchMsg;
  }
  for (auto it = schedulers.begin(); it != schedulers.end(); it++) {
    (*it)->StopSchedule();
  }
}
