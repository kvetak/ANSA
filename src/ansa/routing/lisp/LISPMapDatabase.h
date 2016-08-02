//
// Copyright (C) 2013, 2014 Brno University of Technology
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
 * @author Vladimir Vesely / ivesely@fit.vutbr.cz / http://www.fit.vutbr.cz/~ivesely/
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */

#ifndef __INET_LISPMAPDATABASE_H_
#define __INET_LISPMAPDATABASE_H_

#include <omnetpp.h>
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/common/ModuleAccess.h"
#include "ansa/routing/lisp/LISPCommon.h"
#include "ansa/routing/lisp/LISPMapStorageBase.h"

namespace inet {

class LISPMapDatabase : public cSimpleModule, public LISPMapStorageBase
{
  public:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

    bool isOneOfMyEids(L3Address addr);

  protected:
    IInterfaceTable*    Ift;                ///< Provides access to the interface table.
    bool advertonlyowneids;

    void parseEtrMappings(cXMLElement* config);
    void updateDisplayString();
};

}

#endif
