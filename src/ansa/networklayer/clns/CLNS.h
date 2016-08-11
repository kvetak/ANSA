// Copyright (C) 2012 - 2016 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
 * @file CLNS.h
 * @author Marcel Marek (mailto:imarek@fit.vutbr.cz)
 * @date 5.8.2016
 */


#ifndef __ANSAINET_CLNS_H_
#define __ANSAINET_CLNS_H_

#include <vector>

#include "inet/common/INETDefs.h"

#include "inet/networklayer/contract/INetworkProtocol.h"
#include "ansa/networklayer/isis/ISISMessage_m.h"
#include "inet/common/ProtocolMap.h"
#include "inet/common/queue/QueueBase.h"

#include "ansa/networklayer/clns/CLNSAddress.h"

using namespace omnetpp;

namespace inet {

class IInterfaceTable;

/**
 * TODO - Generated class
 */
class INET_API CLNS : public cSimpleModule
{
  public:
      typedef std::vector<CLNSAddress> CLNSAddressVector;

  private:
      CLNSAddressVector addressVector;




  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

} //namespace

#endif
