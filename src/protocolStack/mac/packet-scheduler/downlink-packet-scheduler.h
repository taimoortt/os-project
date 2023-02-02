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


#ifndef DOWNLINKPACKETSCHEDULER_H_
#define DOWNLINKPACKETSCHEDULER_H_

#include "packet-scheduler.h"
#include <string>

struct SliceContext {
  struct SchedulerAlgo {
    int alpha;
    int beta;
    int epsilon;
    int psi;
    SchedulerAlgo(int _alpha, int _beta, int _epsilon, int _psi) {
      alpha = _alpha;
      beta = _beta;
      epsilon = _epsilon;
      psi = _psi;
    }
  };

public:
  int                             num_slices_ = 1;
  std::vector<int>                user_to_slice_;
  std::vector<double>             weights_;
  std::vector<SchedulerAlgo>      algo_params_;
  std::vector<int>                priority_;
  std::vector<double>             ewma_time_;
};

class DownlinkPacketScheduler: public PacketScheduler {
public:
	DownlinkPacketScheduler(std::string config_fname="");
	virtual ~DownlinkPacketScheduler();

	virtual void SelectFlowsToSchedule ();

	virtual void DoSchedule (void);
	virtual void DoStopSchedule (void);

	virtual void RBsAllocation ();
	virtual double ComputeSchedulingMetric (RadioBearer *bearer, double spectralEfficiency, int subChannel) = 0;

	void UpdateAverageTransmissionRate (void);

  SliceContext  slice_ctx_;

};

#endif /* DOWNLINKPACKETSCHEDULER_H_ */
