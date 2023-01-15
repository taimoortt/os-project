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

#include "fullband-cqi-manager.h"
#include "../NetworkNode.h"
#include "../../core/idealMessages/ideal-control-messages.h"
#include "../../protocolStack/mac/AMCModule.h"
#include "../UserEquipment.h"
#include "../ENodeB.h"
#include "../HeNodeB.h"
#include "../../phy/lte-phy.h"
#include "../../core/spectrum/bandwidth-manager.h"
#include "../../load-parameters.h"


FullbandCqiManager::FullbandCqiManager()
{}

FullbandCqiManager::~FullbandCqiManager()
{}

void
FullbandCqiManager::CreateCqiFeedbacks (std::vector<double> sinr)
{
#ifdef TEST_CQI_FEEDBACKS
  std::cout << "FullbandCqiManager -> CreateCqiFeedbacks " << std::endl;
#endif

#ifdef AMC_MAPPING
  std::cout << "\t sinr: ";
  for (int i = 0; i < sinr.size (); i++)
    {
	  std::cout << sinr.at (i) << " ";
    }
  std::cout << std::endl;
#endif

  UserEquipment* thisNode = (UserEquipment*) GetDevice ();
  NetworkNode* targetNode = thisNode->GetTargetNode ();

  AMCModule *amc = GetDevice ()->GetProtocolStack ()->GetMacEntity ()->GetAmcModule ();
  std::vector<int> cqi = amc->CreateCqiFeedbacks (sinr);

  CqiIdealControlMessage *msg = new CqiIdealControlMessage ();
  msg->SetSourceDevice (thisNode);
  msg->SetDestinationDevice (targetNode);

  int nbSubChannels = cqi.size ();
  std::vector<double> dlSubChannels = thisNode->GetPhy ()->GetBandwidthManager ()->GetDlSubChannels ();

  for (int i = 0; i < nbSubChannels; i++) {
    msg->AddNewRecord (dlSubChannels.at (i), cqi.at (i));
  }

  SetLastSent ();

  thisNode->GetPhy ()->SendIdealControlMessage (msg);
}


void
FullbandCqiManager::CreateCqiFeedbacks (std::vector<CQIRecord> cqi_record)
{
  UserEquipment* thisNode = (UserEquipment*) GetDevice ();
  NetworkNode* targetNode = thisNode->GetTargetNode ();

  AMCModule *amc = GetDevice ()->GetProtocolStack ()->GetMacEntity ()->GetAmcModule ();
  std::vector<double> sinr;
  std::vector<double> sinr_with_mute;
  for (auto it = cqi_record.begin(); it != cqi_record.end(); ++it) {
    sinr.push_back(it->sinr);
    sinr_with_mute.push_back(it->sinr_with_mute);
  }
  std::vector<int> cqi = amc->CreateCqiFeedbacks(sinr);
  std::vector<int> cqi_with_mute = amc->CreateCqiFeedbacks(sinr_with_mute);

  CqiWithMuteIdealControlMessage *msg = new CqiWithMuteIdealControlMessage ();
  msg->SetSourceDevice (thisNode);
  msg->SetDestinationDevice (targetNode);

  int nbSubChannels = cqi.size ();
  std::vector<double> dlSubChannels = thisNode->GetPhy ()->GetBandwidthManager ()->GetDlSubChannels ();

  for (int i = 0; i < nbSubChannels; i++) {
    msg->AddNewRecord (dlSubChannels.at (i), cqi.at (i),
      cqi_with_mute.at(i), cqi_record.at(i).neighbor_cell);
  }

  SetLastSent ();

  thisNode->GetPhy ()->SendIdealControlMessage (msg);
}