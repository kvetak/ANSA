//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
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
 * @authors Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */

#ifndef ANSAETHERMACBASEVF_H_
#define ANSAETHERMACBASEVF_H_

#include "inet/linklayer/ethernet/EtherMACBase.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet{
// Forward declarations:
class EtherFrame;
class EtherTraffic;
class InterfaceEntry;
class ANSA_InterfaceEntry;


class ANSA_EtherMACBaseVF: public EtherMACBase {
  public:
    ANSA_EtherMACBaseVF() {};

  protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void registerInterface();

    /** Checks destination address and drops the frame when frame is not for us; returns true if frame is dropped */
    virtual bool dropFrameNotForUs(EtherFrame *frame) override;

};
}//namespace inet
#endif /* ANSAETHERMACBASEVF_H_ */
