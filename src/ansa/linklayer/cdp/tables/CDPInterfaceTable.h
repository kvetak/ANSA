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
        InterfaceEntry *interface;  // Physical network interface
        CDPTimer *updateTimer;
        bool enabled;
        int fastStart;

    public:
        CDPInterface(InterfaceEntry *iface);
        virtual ~CDPInterface();
        virtual std::string info() const override;

        int getInterfaceId() {return (interface) ? interface->getInterfaceId() : -1;}
        void decFastStart() {fastStart--;}

        // getters
        InterfaceEntry *getInterface() {return interface;}
        CDPTimer* getUpdateTimer() {return updateTimer;}
        bool getEnabled() {return enabled;}
        int getFastStart() {return fastStart;}

        // setters
        void setInterface(InterfaceEntry *i) {interface = i;}
        void setUpdateTimer(CDPTimer* u) {updateTimer = u;}
        void setEnabled(bool e) {enabled = e;}
        void setFastStart() {fastStart = 2;}
};


class CDPInterfaceTable
{
  protected:
    std::vector<CDPInterface *> interfaces;

  public:
    virtual ~CDPInterfaceTable();

    std::vector<CDPInterface *>& getInterfaces() {return interfaces;}
    CDPInterface * findInterfaceById(const int ifaceId);
    void addInterface(CDPInterface * iface);
    void removeInterfaces();
    void removeInterface(CDPInterface * iface);
    void removeInterface(int ifaceId);
};
} /* namespace inet */

#endif /* CDPINTERFACE_H_ */
