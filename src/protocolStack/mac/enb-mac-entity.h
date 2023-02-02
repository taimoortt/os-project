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


#ifndef ENB_MAC_ENTITY_H
#define ENB_MAC_ENTITY_H

#include <list>
#include "mac-entity.h"


/*
 * This class implements the MAC layer of the eNodeB device
 */

class PacketScheduler;
class CqiIdealControlMessage;
class CqiWithMuteIdealControlMessage;
class PdcchMapIdealControlMessage;
class SchedulingRequestIdealControlMessage;

struct CqiReport {
  int cqi;
  int cqi_with_mute;
  int neighbor_cell;  // if neighbor_cell is -1, we don't mute
  int final_cqi;

  CqiReport(int _cqi, int _cqi_with_mute, int _neighbor_cell)
  : cqi(_cqi), cqi_with_mute(_cqi_with_mute),
    neighbor_cell(_neighbor_cell), final_cqi(_cqi){}
};

class EnbMacEntity : public MacEntity
{
public:

  EnbMacEntity (void);
  virtual ~EnbMacEntity (void);


  void SetUplinkPacketScheduler (PacketScheduler* s);
  void SetDownlinkPacketScheduler (PacketScheduler* s);
  PacketScheduler* GetUplinkPacketScheduler (void);
  PacketScheduler* GetDownlinkPacketScheduler (void);

  void ReceiveCqiIdealControlMessage  (CqiIdealControlMessage* msg);
  void ReceiveCqiWithMuteIdealControlMessage (CqiWithMuteIdealControlMessage* msg);
  void SendPdcchMapIdealControlMessage  (PdcchMapIdealControlMessage* msg);
  void ReceiveSchedulingRequestIdealControlMessage (SchedulingRequestIdealControlMessage *msg);
private:

  PacketScheduler* m_uplinkScheduler;
  PacketScheduler* m_downlinkScheduler;
};


#endif /* ENB_MAC_ENTITY_H */
