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

class inet::IPv4Address extends void;
class inet::MACAddress extends void;
class inet::IPv4RoutingDecision extends void;

cplusplus {{
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/common/IPProtocolId_m.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
}}
//
// TODO generated message class
//
class AnsaIPv4RoutingDecision extends inet::IPv4RoutingDecision 
{
	int vforwarderId = -1;
}
