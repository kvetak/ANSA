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

class noncobject IPv4Address;
class noncobject MACAddress;
class noncobject IPv4RoutingDecision;

cplusplus {{
#include "IPv4Address.h"
#include "IPProtocolId_m.h"
#include "MACAddress.h"
#include "IPv4ControlInfo_m.h"
}}
//
// TODO generated message class
//
class AnsaIPv4RoutingDecision extends IPv4RoutingDecision 
{
	int vforwarderId = -1;
}
