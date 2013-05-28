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
#include <vector>
typedef std::vector<IPv6Address> IPVector6;
}}

class noncobject IPVector6;

enum MLDType
{
    MLD_MULTICAST_LISTENER_QUERY = 130;
    MLD_MULTICAST_LISTENER_REPORT = 131;
    MLD_MULTICAST_LISTENER_DONE = 132;
    MLD_MULTICAST_LISTENER_V2REPORT = 140;
}

struct GroupRecordMLD
{
    int recordType;
    int numOfSources;
    IPv6Address groupAddress;
    IPVector6 SourceList;
};

packet MLDMessage
{
    int type enum(MLDType);
    int maxRespDelay;
    IPv6Address groupAddress;
}

packet MLDv2Query
{
    int type enum(MLDType) = MLD_MULTICAST_LISTENER_QUERY;
    int maxRespCode;
    IPv6Address groupAddress;
    int numOfSources;
    IPVector6 sourceList;
}

packet MLDv2Report
{
    int type enum(MLDType) = MLD_MULTICAST_LISTENER_V2REPORT;
    int numGroupRecords;
    GroupRecordMLD groupRecord[];
}