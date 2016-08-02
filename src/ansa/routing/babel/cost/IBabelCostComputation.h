//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
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
* @file IBabelCostComputation.h
* @author Vit Rek (rek@kn.vutbr.cz)
* @author Vladimir Vesely (ivesely@fit.vutbr.cz)
* @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
* @brief Link cost computation module
* @detail Abstract class defines required methods
*/

#ifndef IBABELCOSTCOMPUTATION_H_
#define IBABELCOSTCOMPUTATION_H_

namespace inet {
const uint16_t BABEL_COST_INFINITY = 0xFFFF;

/**
 * Cost computation module
 *
 * Expects history vector represented as uint16_t, most recent interval saved as most significant bit
 */
class IBabelCostComputation
{
public:
    virtual uint16_t computeRxcost(uint16_t history, uint16_t nominalrxcost) = 0;
    virtual uint16_t computeCost(uint16_t history, uint16_t nominalrxcost, uint16_t txcost) = 0;
};

}
#endif /* IBABELCOSTCOMPUTATION_H_ */
