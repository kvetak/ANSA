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
* @file IBabelCostComputation.h
* @author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
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
