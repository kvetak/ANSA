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
