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



#ifndef FRAMEMANAGER_H_
#define FRAMEMANAGER_H_

#include <iostream>
#include "NetworkManager.h"
#include "../core/eventScheduler/simulator.h"
#include "../protocolStack/mac/packet-scheduler/downlink-packet-scheduler.h"
#include "TDDConfiguration.h"


 /*
   * LTE Frame Structure:
   *
   *  ** Frame structure type 1 (FS1): FDD
   *    ...
   *  ** Frame structure type 2 (FS2): TDD
   *    ...
   */


class FrameManager {
public:
	enum FrameStructure
	  {
		FRAME_STRUCTURE_FDD,
		FRAME_STRUCTURE_TDD
	  };
private:
	FrameStructure m_frameStructure;

	int m_TDDFrameConfiguration;

	int m_nbFrames;
	int m_nbSubframes;
	unsigned long m_TTICounter;

	FrameManager();
	static FrameManager *ptr;

public:
	//FrameManager();
	virtual ~FrameManager();

	static FrameManager*
	Init (void)
	  {
		if (ptr==NULL)
	      {
		    ptr = new FrameManager;
	   	  }
		return ptr;
	  }

	void
	SetFrameStructure (FrameStructure frameStructure);
	FrameStructure
	GetFrameStructure (void) const;

	void
	SetTDDFrameConfiguration (int configuration);
	int
	GetTDDFrameConfiguration (void) const;

	int
	GetSubFrameType (int nbSubFrame);

	void
	UpdateNbFrames (void);
	int
	GetNbFrames (void) const;
	void
	UpdateNbSubframes (void);
	void
	ResetNbSubframes (void);
	int
	GetNbSubframes (void) const;
	void
	UpdateTTIcounter (void);
	unsigned long
	GetTTICounter (void) const;

	void
	Start (void);
	void
	StartFrame (void);
	void
	StopFrame (void);
	void
	StartSubframe (void);
	void
	StopSubframe (void);

	NetworkManager* GetNetworkManager (void);

	void UpdateUserPosition(void);
	void ResourceAllocation(void);

  void CentralResourceAllocation(void);

  void CentralDownlinkRBsAllocation(void);
  bool TuneWeightsAcrossCells(std::vector<DownlinkPacketScheduler*>&);
  void NVSAllocateOneRB(
    std::vector<DownlinkPacketScheduler*>& schedulers,
    int rb_id);

  void RadioSaberAllocateOneRB(
    std::vector<DownlinkPacketScheduler*>& schedulers,
    int rb_id);

  void FinalizeAllocation(
    std::vector<int>& cell_byorder,
    std::vector<DownlinkPacketScheduler*>& schedulers,
    std::vector<FlowToSchedule*>& cell_flows,
    int rb_id);
  
  std::vector<int> slice_offset_;
  void RadioSaberAllocateOneRBGlobal(
    std::vector<DownlinkPacketScheduler*>& schedulers,
    int rb_id, std::vector<int>& quota);  
};

#endif /* FRAMEMANAGER_H_ */
