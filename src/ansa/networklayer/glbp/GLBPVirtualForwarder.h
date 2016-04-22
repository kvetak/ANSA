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
        VFState state;
        int priority;
        int weight;

    public:
        //default cisco values;
        GLBPVirtualForwarder() { state = DISABLED; priority = 167; weight = 100; };
        GLBPVirtualForwarder(VFState s, int p, int w) { state = s; priority = p; weight = w;};
        virtual ~GLBPVirtualForwarder() {};
        /** @name Field getters. Note they are non-virtual and inline, for performance reasons. */
        //@{
        int getState() { return state; };
        int getPriority() { return priority; };
        bool getWeight() { return weight; };
        //@}

        /** @name Field setters */
        //@{
        virtual void setState(VFState s) { state = s; };
        virtual void setPriority(int p) { priority = p; };
        virtual void setWeight(int w) { weight = w; };
        //@}

};

} /* namespace inet */

#endif /* GLBPVIRTUALFORWARDER_H_ */
