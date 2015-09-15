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
 * Copyright (C) 2012 - 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
 *
 * @file RBridgeSplitter.h
 * @author Marcel Marek (mailto:xscrew02@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 10.2.2013
 * @brief Handles de/encapsulation for IS-IS module and splits traffic between IS-IS and TRILL modules.
 * @detail Handles de/encapsulation for IS-IS module and splits traffic between IS-IS and TRILL modules.
 * @todo Z9
 */

#ifndef RBRIDGESPLITTER_H_
#define RBRIDGESPLITTER_H_

#include <csimplemodule.h>
#include "ansa/networklayer/isis/ISISAccess.h"
#include "ansa/linklayer/rbridge/TRILLAccess.h"
#include "ansa/linklayer/rbridge/RBVLANTable.h"
//#include "RBPortTable.h"
#include "ansa/linklayer/switch/AnsaEtherFrame_m.h"
#include "networklayer/contract/IInterfaceTable.h"
#include "networklayer/common/InterfaceTableAccess.h"


class RBridgeSplitter : public cSimpleModule
{
//    public:
//        RBridgeSplitter();
//        virtual ~RBridgeSplitter();
    private:
        ISIS *isisModule;
        TRILL *trillModule;
        RBVLANTable *vlanTableModule;
//        RBPortTable *portTableModule;
        inet::IInterfaceTable *ift;
    protected:
      virtual void initialize(int stage);
      virtual int numInitStages() const
      {
          return 4;
      }
      virtual void handleMessage(cMessage *msg);
};

#endif /* RBRIDGESPLITTER_H_ */
