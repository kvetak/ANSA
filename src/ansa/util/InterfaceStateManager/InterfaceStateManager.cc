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

#include "ansa/util/InterfaceStateManager/InterfaceStateManager.h"
#include "networklayer/common/InterfaceTableAccess.h"

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
      inet::InterfaceEntry *targetInt = ift->getInterfaceByName(node.getAttribute("int"));
      EV << "interface " << node.getAttribute("int") << " is going DOWN" << endl;
      if (targetInt == NULL){
         throw cRuntimeError("Interface %s not found", node.getAttribute("int"));
      }
      changeInterfaceState(targetInt, true);
    }
    else if(!strcmp(node.getTagName(), "interfaceup"))
    {
      inet::InterfaceEntry *targetInt = ift->getInterfaceByName(node.getAttribute("int"));
      EV << "interface " << node.getAttribute("int") << " is going UP" << endl;
      if (targetInt == NULL){
         throw cRuntimeError("Interface %s not found", node.getAttribute("int"));
      }
      changeInterfaceState(targetInt, false);
    }
    else if(!strcmp(node.getTagName(), "interfaceconfig"))
    {
        processInterfaceConfigCommand(node);
    }
    else
        ASSERT(false);

}
/*
 * Method changes the interface state to desired value
 * @param targetInt     - interface, that is changing the state
 * @param toDown        - new interface state
 */

void InterfaceStateManager::changeInterfaceState(inet::InterfaceEntry *targetInt, bool toDown)
{
  Enter_Method_Silent();
  
  if(targetInt != NULL)
  {
    bool isDown = (targetInt->getState()==inet::InterfaceEntry::DOWN);
    if(!isDown && toDown)
      targetInt->setState(inet::InterfaceEntry::DOWN);
    /*
     * Migration towards ANSAINET2.2
     */
     else
      if(isDown && !toDown)
        targetInt->setState(inet::InterfaceEntry::UP);
  }
  else
    EV << "Interface NOT found\n";
}

/**
 *  Change of interface bandwidth (in kbps)
 *      <interfaceconfig bandwidth="1544" module="R1.interfaceStateManager" int="eth0"/>
 *  Change of interface delay (in tens of microseconds)
 *      <interfaceconfig delay="2000" module="R1.interfaceStateManager" int="eth0"/>
 */
void InterfaceStateManager::processInterfaceConfigCommand(const cXMLElement& node)
{
    inet::InterfaceEntry *targetInt = ift->getInterfaceByName(node.getAttribute("int"));
    if (targetInt == NULL)
        throw cRuntimeError("Interface %s not found", node.getAttribute("int"));

    std::stringstream os;
    int ifParam;

    if (node.getAttribute("bandwidth") != NULL)
    {
        EV << "Interface " << node.getAttribute("int") << " change bandwidth" << endl;
        os << node.getAttribute("bandwidth");
        if (!(os >> ifParam))
            throw cRuntimeError("Bad value for bandwidth on interface %s", node.getAttribute("int"));
        targetInt->setBandwidth(ifParam);
    }
    else if (node.getAttribute("delay") != NULL)
    {
        EV << "Interface " << node.getAttribute("int") << " change delay" << endl;
        os << node.getAttribute("delay");
        if (!(os >> ifParam))
            throw cRuntimeError("Bad value for delay on interface %s", node.getAttribute("int"));
        targetInt->setDelay(ifParam * 10);
    }
}
