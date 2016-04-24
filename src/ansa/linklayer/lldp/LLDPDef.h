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

#ifndef LLDPDEF_H_
#define LLDPDEF_H_

#include <omnetpp.h>

namespace inet {


enum AS
{
    enabledRxTx = 0,         // is enabled for reception and transmission of LLDPDUs
    enabledTxOnly = 1,       // is enabled for transmission of LLDPDUs only
    enabledRxOnly = 2,       // is enabled for reception of LLDPDUs only
    disabled = 3             // is disabled for both reception and transmission
};

struct LLDPTlvOrgSpec
{
    uint32_t    oui;            // organizationally unique identifier
    uint8_t     subtype;        // organizationally defined subtype
    std::string value;          // organizationally defined information string
};

} /* namespace inet */
/*
class LLDPSystemInfo
{
    std::string chassisId;

};*/

#endif /* LLDPDEF_H_ */
