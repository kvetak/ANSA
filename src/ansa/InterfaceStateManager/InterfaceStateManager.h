//
// Copyright (C) 2009 Martin Danko
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef INTERFACESTATEMANAGER_H
#define INTERFACESTATEMANAGER_H

#include <omnetpp.h>

#include "IInterfaceTable.h"
#include "IScriptable.h"

/**
 *  This module allows managing changes of interface states in AnsaRouter.
 *  It can receive XML command from ScenarioManger and accomplish all necessary steps.
 *  Now, it supports 2 command:
 *  1.  Change of interface state to UP
 *      <interfaceup module="R1.interfaceStateManager" int="eth0"/>
 *  2.  Change of interface state to DOWN
 *      <interfacedown module="R1.interfaceStateManager" int="eth0"/>
 *
 */
 
class INET_API InterfaceStateManager : public cSimpleModule, public IScriptable
{
  private:
  
  IInterfaceTable*     ift;        ///< Provides access to the interface table.

  protected:
  
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    // IScriptable implementation
    virtual void processCommand(const cXMLElement& node);
    
  public:
  
    void changeInterfaceState(InterfaceEntry *targetInt, bool toDown);

};

#endif
