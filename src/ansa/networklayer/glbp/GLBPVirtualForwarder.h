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
* @file GLBPVirtualForwarder.h
* @author Jan Holusa
* @brief
* @detail
*/
#ifndef GLBPVIRTUALFORWARDER_H_
#define GLBPVIRTUALFORWARDER_H_

#include "ansa/networklayer/common/VirtualForwarder.h"

namespace inet {

class GLBPVirtualForwarder : public VirtualForwarder
{
public:
    enum VFState{
        DISABLED=1,
        INIT = 2,
        LISTEN = 4,
        ACTIVE = 32,
    };

    protected:
        int state;
        int priority;
        int weight;
        int forwarder;
        bool AVG;   //need for ARP response
//        bool token;
        bool available;
        MACAddress *primaryRouter = nullptr;

//    protected:
//        std::string intToVfState(int state);

    public:
        //default cisco values;
        GLBPVirtualForwarder() { state = DISABLED; priority = 135; weight = 100; disable = true; forwarder = 0; AVG = false;  available = false;};
        GLBPVirtualForwarder(int s, int p, int w) { state = s; priority = p; weight = w; disable = true; forwarder = 0; AVG = false; available = false;};
        virtual ~GLBPVirtualForwarder() {};
        /** @name Field getters. Note they are non-virtual and inline, for performance reasons. */
        //@{
        int getState() { return state; };
        int getPriority() { return priority; };
        bool getWeight() { return weight; };
        int getForwarder() { return forwarder; };
        const MACAddress *getPrimary() {return primaryRouter; };
        bool isAVG() { return AVG; };
        bool isAvailable() { return available; };
        //@}

        /** @name Field setters */
        //@{
        virtual void setState(int s) { state = s; };
        virtual void setPriority(int p) { priority = p; };
        virtual void setWeight(int w) { weight = w; };
        virtual void setForwarder (int n) { forwarder = n; };
        virtual void setPrimary (MACAddress *mac) {primaryRouter = mac; };
        virtual void enableAVG() {AVG = true;};
        virtual void disableAVG() {AVG = false;};
        virtual void setAvailable(bool val) {available = val;};
//        virtual void disableToken() {token = false;};
        //@}

        //statistics
        virtual std::string info() const;

};

} /* namespace inet */

#endif /* GLBPVIRTUALFORWARDER_H_ */
