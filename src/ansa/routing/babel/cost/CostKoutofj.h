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
* @file CostKoutofj.h
* @author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
* @brief Link cost computation module
* @detail Computes link cost using k-out-of-j method
*/

#ifndef __ANSA_COSTKOUTOFJ_H_
#define __ANSA_COSTKOUTOFJ_H_

#include "inet/common/INETDefs.h"
#include "ansa/routing/babel/cost/IBabelCostComputation.h"
namespace inet {
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
}
#endif
