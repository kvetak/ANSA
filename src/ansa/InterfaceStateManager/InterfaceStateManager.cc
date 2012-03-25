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

#include "InterfaceStateManager.h"
#include "InterfaceTableAccess.h"

Define_Module(InterfaceStateManager);

void InterfaceStateManager::initialize()
{
    ift = InterfaceTableAccess().get();
    if (ift == NULL){
       throw cRuntimeError("AnsaInterfaceTable not found");
    }
}

void InterfaceStateManager::handleMessage(cMessage *msg)
{
    ASSERT(false);
}

/*
 * Method processes the XML command received from ScenarioManager
 * @param node 		- XML element with command from ScenarioManager
 */

void InterfaceStateManager::processCommand(const cXMLElement& node)
{
   if (!strcmp(node.getTagName(), "interfacedown"))
    {
      InterfaceEntry *targetInt = ift->getInterfaceByName(node.getAttribute("int"));
      EV << "interface " << node.getAttribute("int") << " is going DOWN" << endl;
      if (targetInt == NULL){
         throw cRuntimeError("Interface %s not found", node.getAttribute("int"));
      }
      changeInterfaceState(targetInt, true);
    }
    else if(!strcmp(node.getTagName(), "interfaceup"))
    {
      InterfaceEntry *targetInt = ift->getInterfaceByName(node.getAttribute("int"));
      EV << "interface " << node.getAttribute("int") << " is going UP" << endl;
      if (targetInt == NULL){
         throw cRuntimeError("Interface %s not found", node.getAttribute("int"));
      }
      changeInterfaceState(targetInt, false);
    }
    else
        ASSERT(false);

}
/*
 * Method changes the interface state to desired value
 * @param targetInt     - interface, that is changing the state
 * @param toDown        - new interface state
 */

void InterfaceStateManager::changeInterfaceState(InterfaceEntry *targetInt, bool toDown)
{
  Enter_Method_Silent();
  
  if(targetInt != NULL)
  {
    bool isDown = targetInt->isDown();
    if(!isDown && toDown)
      targetInt->setDown(true);
    else
      if(isDown && !toDown)
        targetInt->setDown(false);
  }
  else
    EV << "Interface NOT found\n";
}

