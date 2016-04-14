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

#include "CDPTableEntry.h"

namespace inet {

CDPTableEntry::CDPTableEntry() {
    // TODO Auto-generated constructor stub

}

CDPTableEntry::~CDPTableEntry() {
    // TODO Auto-generated destructor stub
}

std::string CDPTableEntry::info() const
{
    std::stringstream out;

    out << name << ", local int: " << interface->getName();
    out << ", holdtime: " << round(ttl.dbl()-(simTime()-lastUpdate).dbl()) << ", cap: " << capabilities;
    out << ", send int: " << portSend;
    return out.str();
}

} /* namespace inet */
