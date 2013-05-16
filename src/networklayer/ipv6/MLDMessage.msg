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

class noncobject IPv6Address;

cplusplus {{
#include "INETDefs.h"
#include "IPv6Address.h"
}}

enum MLDType
{
    MLD_MEMBERSHIP_QUERY = 0x11;
    MLD_MEMBERSHIP_REPORT = 0x16;
    MLD_LEAVE_GROUP = 0x17;
}

packet MLDMessage
{
    int type enum(MLDType);
    int maxRespTime;
    IPv6Address groupAddress;
}