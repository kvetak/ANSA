// The MIT License (MIT)
//
// Copyright (c) 2016 Brno University of Technology
//
//@author Vladimir Vesely (iveselyATfitDOTvutbrDOTcz)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

/**
* @file CostEtx.cc
* @author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
* @brief Link cost computation module
* @detail Computes link cost using ETX method
*/

#include "ansa/routing/babel/cost/CostEtx.h"
namespace inet {
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
}
