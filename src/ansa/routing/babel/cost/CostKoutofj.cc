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
* @file CostKoutofj.cc
* @author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
* @brief Link cost computation module
* @detail Computes link cost using k-out-of-j method
*/

#include "ansa/routing/babel/cost/CostKoutofj.h"
namespace inet {
Define_Module(CostKoutofj);

void CostKoutofj::initialize()
{
    int tmpk = par("k");
    int tmpj = par("j");

    if(tmpk > 0)
    {
        k = tmpk;
    }
    else
    {
        throw cRuntimeError("Bad value for parameter k - must be > 0");
    }

    if(tmpj >= tmpk)
    {
        j = tmpj;
    }
    else
    {
        throw cRuntimeError("Bad value for parameter j - must be >= k");
    }

}

void CostKoutofj::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module does not process messages");
}

/**
 * RXCOST computation using k-out-of-j
 *
 * @param   history vector as uint16_t
 * @param   nominalrxcost   rxcost in ideal conditions
 * @return  RXCOST
 */
uint16_t CostKoutofj::computeRxcost(uint16_t history, uint16_t nominalrxcost)
{
    ASSERT(k > 0);
    ASSERT(j >= k);
    ASSERT(nominalrxcost >= 1);

    unsigned int reccount = 0;

    for(unsigned int i = 0; i < j; ++i)
    {// count correctly received in last j
        if((history << i) & 0x8000)
        {
            ++reccount;
        }
    }

    if(reccount >= k)
    {
        return nominalrxcost;
    }
    else
    {
        return BABEL_COST_INFINITY;
    }
}

/**
 * Cost computation using k-out-of-j method
 *
 * @param   history vector as uint16_t
 * @param   nominalrxcost
 * @param   txcost
 * @return  computed cost
 */
uint16_t CostKoutofj::computeCost(uint16_t history, uint16_t nominalrxcost, uint16_t txcost)
{
    uint16_t cost = BABEL_COST_INFINITY;

    if(computeRxcost(history, nominalrxcost) != BABEL_COST_INFINITY)
    {// rxcost is not INFINITY -> set cost as txcost
        cost = txcost;
    }

    return cost;
}
}
