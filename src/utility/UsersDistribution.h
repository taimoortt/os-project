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


#ifndef USERSDISTRIBTION_H_
#define USERSDISTRIBTION_H_

#include "../core/cartesianCoodrdinates/CartesianCoordinates.h"
#include "CellPosition.h"
#include "../componentManagers/NetworkManager.h"

#include <vector>
#include <iostream>
#include <time.h>

static CartesianCoordinates*
GetCartesianCoordinatesFromPolar (double r, double angle)
{
  double x = r * cos (angle);
  double y = r * sin (angle);

  CartesianCoordinates *coordinates = new CartesianCoordinates ();
  coordinates->SetCoordinates(x,y);
  return coordinates;
}

static vector<CartesianCoordinates*>*
GetUniformUsersDistribution (int idCell, int nbUE)
{
  NetworkManager * networkManager = NetworkManager::Init();
  vector<CartesianCoordinates*> *vectorOfCoordinates = new vector<CartesianCoordinates*>;

  int n_cells = networkManager->GetCellContainer()->size();
  Cell *cell = networkManager->GetCellByID(idCell);
  double cell_x = cell->GetCellCenterPosition()->GetCoordinateX();
  double cell_y = cell->GetCellCenterPosition()->GetCoordinateY();
  Cell *cell_neighbor = networkManager->GetCellByID((idCell+1) % n_cells);
  double cell_x_neighbor = cell_neighbor->GetCellCenterPosition()->GetCoordinateX();
  double cell_y_neighbor = cell_neighbor->GetCellCenterPosition()->GetCoordinateY();
  double midpoint_x = ((cell_x + cell_x_neighbor)/2);
  double midpoint_y = ((cell_y + cell_y_neighbor)/2);

  CartesianCoordinates *cellCoordinates = cell->GetCellCenterPosition();
  double r; double angle;

  // for (int i = 0; i < nbUE; i++)
  // {
  //   // r = (double) (rand() % (int)(cell->GetRadius()*1000) * 2.732);
  //   r = (double) (rand() % (int)(cell->GetRadius()*1000) * 4);
  //   angle = (double)(rand() %360) * ((2*3.14)/360);
  //   CartesianCoordinates *newCoordinates = GetCartesianCoordinatesFromPolar (r, angle);
  //   //Compute absoluteCoordinates
  //   vectorOfCoordinates->push_back(newCoordinates);
  // }

  for (int i = 0; i < nbUE; i++)
  {
    double random_num = (double)(rand() % 60);
    CartesianCoordinates *newCoordinates = new CartesianCoordinates(
      midpoint_x + random_num, midpoint_y + random_num);
	  vectorOfCoordinates->push_back(newCoordinates);
  }

  return vectorOfCoordinates;
}

static vector<CartesianCoordinates*>*
GetUniformUsersDistributionInFemtoCell (int idCell, int nbUE)
{
  NetworkManager * networkManager = NetworkManager::Init();
  vector<CartesianCoordinates*> *vectorOfCoordinates = new vector<CartesianCoordinates*>;

  Femtocell *cell = networkManager->GetFemtoCellByID(idCell);

  double side = cell->GetSide();

  CartesianCoordinates *cellCoordinates = cell->GetCellCenterPosition();
  double r; double angle;

  for (int i = 0; i < nbUE; i++)
    {
	  r = (double)(rand() %(int)side);
	  angle = (double)(rand() %360) * ((2*3.14)/360);

	  CartesianCoordinates *newCoordinates = GetCartesianCoordinatesFromPolar (r, angle);

	  //Compute absoluteCoordinates
	  newCoordinates->SetCoordinateX (cellCoordinates->GetCoordinateX () + newCoordinates->GetCoordinateX ());
	  newCoordinates->SetCoordinateY (cellCoordinates->GetCoordinateY () + newCoordinates->GetCoordinateY ());

	  vectorOfCoordinates->push_back(newCoordinates);
    }

  return vectorOfCoordinates;
}


#endif /* USERSDISTRIBTION_H_ */
