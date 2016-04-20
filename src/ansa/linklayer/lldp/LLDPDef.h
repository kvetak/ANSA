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

namespace inet {


enum AS
{
    enabledRxTx = 0,         // int is enabled for reception and transmission of LLDPDUs
    enabledTxOnly = 1,       // int is enabled for transmission of LLDPDUs only
    enabledRxOnly = 2,       // int is enabled for reception of LLDPDUs only
    disabled = 3             // int is disabled for both reception and transmission
};
} /* namespace inet */

#endif /* LLDPDEF_H_ */
