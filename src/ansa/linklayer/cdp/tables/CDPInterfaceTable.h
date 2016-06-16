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
* @file CDPInterfaceTable.h
* @author Tomas Rajca
*/

#ifndef CDPINTERFACE_H_
#define CDPINTERFACE_H_

#include "ansa/linklayer/cdp/CDPTimer_m.h"
#include "inet/networklayer/common/InterfaceEntry.h"

namespace inet {

/**
 * Class holding informatation about interface that are capable
 * to run CDP.
 */
class INET_API CDPInterface : public cObject {
        friend class CDPInterfaceTable;
    protected:
        InterfaceEntry *interface;  // Physical network interface
        CDPTimer *updateTimer;
        bool CDPEnabled;
        int fastStart;

    public:
        CDPInterface(InterfaceEntry *iface);
        virtual ~CDPInterface();
        virtual std::string info() const override;
        friend std::ostream& operator<<(std::ostream& os, const CDPInterface& e)
        {
            return os << e.info();
        }

        int getInterfaceId() {return (interface) ? interface->getInterfaceId() : -1;}
        void decFastStart() {fastStart--;}

        // getters
        InterfaceEntry *getInterface() {return interface;}
        CDPTimer* getUpdateTimer() {return updateTimer;}
        bool isCDPEnabled() {return CDPEnabled;}
        int getFastStart() {return fastStart;}

        // setters
        void setInterface(InterfaceEntry *i) {interface = i;}
        void setUpdateTimer(CDPTimer* u) {updateTimer = u;}
        void setCDPEnabled(bool e) {CDPEnabled = e;}
        void setFastStart() {fastStart = 2;}
};

/**
 * Class holding informatation about interfaces that are capable
 * to run CDP.
 */
class INET_API CDPInterfaceTable : public cSimpleModule
{
  protected:
    std::vector<CDPInterface *> interfaces;

    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *) override;

  public:
    virtual ~CDPInterfaceTable();

    std::vector<CDPInterface *>& getInterfaces() {return interfaces;}

    /**
     * Returns the interface with specified interface ID.
     *
     * @param   ifaceId    interface ID
     * @return  cdp interface
     */
    CDPInterface * findInterfaceById(int ifaceId);

    /**
     * Adds the a interface to the table. The operation might fail
     * if the interface is already in the table.
     *
     * @param   iface   interface to add
     */
    void addInterface(CDPInterface * iface);

    /**
     * Remove all interfaces from the table.
     */
    void removeInterfaces();

    /**
     * Remove a interface from the table.
     * If the interface was not found in the table then it is untouched,
     * otherwise deleted.
     *
     * @param   iface   Interface to delete
     */
    void removeInterface(CDPInterface * iface);

    /**
     * Remove a interface identified by interface ID from the table.
     * If the interface was not found in the table then it is untouched,
     * otherwise deleted.
     *
     * @param   ifaceId ID of interface to delete
     */
    void removeInterface(int ifaceId);
};
} /* namespace inet */

#endif /* CDPINTERFACE_H_ */
