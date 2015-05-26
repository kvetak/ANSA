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
* @file CostEtx.cc
* @author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
* @brief Link cost computation module
* @detail Computes link cost using ETX method
*/

#include "CostEtx.h"

Define_Module(CostEtx);

void CostEtx::initialize()
{
}

void CostEtx::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module does not process messages");
}

/**
 * RXCOST computation using history rating
 *
 * @param   history vector as uint16_t
 * @param   nominalrxcost   rxcost in ideal conditions
 * @return  RXCOST
 */
uint16_t CostEtx::computeRxcost(uint16_t history, uint16_t nominalrxcost)
{
   // rate history -> last 3 intervals have same values, older intervals count with decreasing values
   unsigned int histrate = ((history & 0x8000) >> 2) +
                           ((history & 0x4000) >> 1) +
                           (history & 0x3FFF);

   unsigned int rxcost = (0x8000 * nominalrxcost) / (histrate + 1);    //if there is no loss in history, then (histrate + 1)=0x8000
   if(rxcost >= BABEL_COST_INFINITY)
   {
       return BABEL_COST_INFINITY;
   }

   return (rxcost & 0xFFFF);
}

/**
 * Cost computation using ETX method
 *
 * @param   history vector as uint16_t
 * @param   nominalrxcost
 * @param   txcost
 * @return  computed cost
 */
uint16_t CostEtx::computeCost(uint16_t history, uint16_t nominalrxcost, uint16_t txcost)
{
    uint16_t cost = BABEL_COST_INFINITY;

    uint16_t rxcost = computeRxcost(history, nominalrxcost);

    if(rxcost >= BABEL_COST_INFINITY)
    {
       return BABEL_COST_INFINITY;
    }

    if(txcost < 256 && rxcost < 256)
    {// in both directions are expected probabilities greater than 100%
       cost = txcost;
    }
    else
    {
       unsigned int a = std::max(txcost, static_cast<uint16_t>(256));     // a = txcost = 256/alpha
       unsigned int b = std::max(rxcost, static_cast<uint16_t>(256));     // b = rxcost = 256/beta

       cost = (a * b) >> 8;            // cost = (MAX(txcost, 256) * rxcost) / 256
    }

    return cost;
}
