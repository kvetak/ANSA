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

#ifndef CDPINTERFACE_H_
#define CDPINTERFACE_H_

#include "ansa/linklayer/cdp/CDPTimer_m.h"
#include "inet/networklayer/common/InterfaceEntry.h"

namespace inet {

class CDPInterface : public cObject {
    protected:
        InterfaceEntry *interface;  ///< Physical network interface
        CDPTimer *updateTimer;
        bool enabled;

    public:
        CDPInterface() :
            interface(NULL),
            updateTimer(NULL),
            enabled(true){}
        CDPInterface(InterfaceEntry *iface, CDPTimer *update) :
            interface(iface),
            updateTimer(update),
            enabled(true){}
        virtual ~CDPInterface();

        int getInterfaceId() {return (interface) ? interface->getInterfaceId() : -1;}

        InterfaceEntry *getInterface() {return interface;}
        void setInterface(InterfaceEntry *i) {interface = i;}

        CDPTimer* getUpdateTimer() {return updateTimer;}
        void setUpdateTimer(CDPTimer* u) {updateTimer = u;}

        bool getEnabled() {return enabled;}
        void setEnabled(bool e) {enabled = e;}
};


/**
 * TODO - Generated class
 */
class CDPInterfaceTable
{
  protected:
    std::vector<CDPInterface *> interfaces;

  public:
    virtual ~CDPInterfaceTable();

    std::vector<CDPInterface *>& getInterfaces() {return interfaces;}
    CDPInterface * findInterfaceById(const int ifaceId);
    CDPInterface * addInterface(CDPInterface * iface);
    void removeInterfaces();
    void removeInterface(CDPInterface * iface);
    void removeInterface(int ifaceId);
    std::string printStats();

};
} /* namespace inet */

#endif /* CDPINTERFACE_H_ */
