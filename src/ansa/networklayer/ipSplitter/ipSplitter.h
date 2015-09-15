// Copyright (C) 2011 - 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
 * @file ipSplitter.h
 * @author Marek Cerny, Matej Hrncirik, Marcel Marek (mailto:xscrew02@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 2011
 * @brief Splits transport protocols to appropriates modules
 * @detail Splits transport protocols to appropriates modules
 */

#ifndef __ANSAINET_IPSPLITTER_H_
#define __ANSAINET_IPSPLITTER_H_

#include <omnetpp.h>
#include "networklayer/ipv6/IPv6Datagram.h"
#include "ansa/networklayer/isis/ISIS.h"
#include "linklayer/common/Ieee802Ctrl.h"

class IpSplitter : public cSimpleModule
{
  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
};

#endif
