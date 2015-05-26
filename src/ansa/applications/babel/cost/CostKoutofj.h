//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 
/**
* @file CostKoutofj.h
* @author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
* @brief Link cost computation module
* @detail Computes link cost using k-out-of-j method
*/

#ifndef __ANSA_COSTKOUTOFJ_H_
#define __ANSA_COSTKOUTOFJ_H_

#include <omnetpp.h>

#include "IBabelCostComputation.h"
/**
 * TODO - Generated class
 */
class CostKoutofj : public cSimpleModule, public IBabelCostComputation
{
  protected:
    unsigned int k;
    unsigned int j;

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

  public:
    virtual uint16_t computeRxcost(uint16_t history, uint16_t nominalrxcost);
    virtual uint16_t computeCost(uint16_t history, uint16_t nominalrxcost, uint16_t txcost);
};

#endif
