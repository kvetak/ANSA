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

//
// TODO generated message class
//

cplusplus {{
#include "ansaOspfCommon6.h"
#include "IPv6Address.h"
using namespace AnsaOspf6;
}}

class noncobject RouterID;
class noncobject DesignatedRouterID;
class noncobject AreaID;
class noncobject IfaceID;
class noncobject InstanceID;
class noncobject IPv6Address;


// === options and support structures =========================================

// http://tools.ietf.org/html/rfc5340#appendix-A.3
enum OspfPacketType6 {
   HelloPacket = 1;
   DatabaseDescriptionPacket = 2;
   LinkStateRequestPacket = 3;
   LinkStateUpdatePacket = 4;
   LinkStateAcknowledgementPacket = 5;
}

// http://tools.ietf.org/html/rfc5340#appendix-A.4
enum LsaType6 {
   RouterLsaType = 1;
   NetworkLsaType = 2;
   InterAreaPrefixLsaType = 3;
   InterAreaRouterLsaType = 4;
   AsExternalLsaType = 5;
   NssaLsaType = 7;	// not implemented
   LinkLsaType = 8;
   IntraAreaPrefixLsaType = 9;
}

enum LinkType6 {
   PointToPointLink = 1;
   TransitLink = 2;
   VirtualLink = 4;
}

class Link6 {
   uint8 type enum(LinkType6) = PointToPointLink;
   uint16 metric;
   IfaceID interfaceID;
   IfaceID neighborInterfaceID;
   RouterID neighborRouterID;
}

struct OspfPrefixOptions6 {
   bool NU_NoUnicast = false;
   bool LA_LocalAddress;
}

class OspfPrefix6 {
   uint8 prefixLenght;
   OspfPrefixOptions6 prefixOptions;
   uint16 metric;
   IPv6Address addressPrefix;
}

// http://tools.ietf.org/html/rfc5340#appendix-A.2
struct OspfOptions6 {
   bool V6_IPv6Routing = true;
   bool E_ExternalRoutingCapability;
   bool N_NSSA = false;
   bool R_Router = true;
   bool DC_DemandCircuits;
}

struct OspfDdOptions6 {
   bool I_Init;
   bool M_More;
   bool MS_MasterSlave;
}

struct OspfLsaRequest6 {
   uint16 lsType;
   uint32 linkStateID;
   RouterID advertisingRouter;
}






// === LSAs ===================================================================
// http://tools.ietf.org/html/rfc5340#appendix-A.4

class OspfLsaHeader6 {
   uint16 lsAge = 0;
   uint16 lsType enum(LsaType6) = RouterLsaType;
   uint32 linkStateID;
   RouterID advertisingRouter;
   uint32 lsSequenceNumber = 0;
}

class OspfLsa6 {
   OspfLsaHeader6 header;
}

class OspfRouterLsa6 extends OspfLsa6 {
   bool V_VirtualLinkEndpoint = false;
   bool E_AsBoundaryRouter = false;
   bool B_AreaBorderRouter = false;
   OspfOptions6 options;
   Link6 links[];
}

class OspfNetworkLsa6 extends OspfLsa6 {
   OspfOptions6 options;
   RouterID attachedRouters[];
}

class OspfInterAreaPrefixLsa6 extends OspfLsa6 {
   uint16 metric;
   uint8 prefixLenght;
   OspfPrefixOptions6 prefixOptions;
   IPv6Address addressPrefix; 
}

class OspfInterAreaRouterLsa6 extends OspfLsa6 {
   OspfOptions6 options;
   uint16 metric;
   RouterID destinationRouterID;
}

class OspfAsExternalLsa6 extends OspfLsa6 {
   bool E_ExternalMetricType = false;
   bool F_ForwardingAddress = false;
   bool T_ExternalRouteTag = false;
   uint16 metric;
   uint8 prefixLenght;
   OspfPrefixOptions6 prefixOptions;
   uint16 referencedLsType enum(LsaType6) = 0;	// RFC 5340 - future use
   IPv6Address addressPrefix;
   IPv6Address forwardingAddress;
   uint32 externalRouteTag;
   uint32 referencedLinkStateID;				// RFC 5340 - future use
}

class OspfLinkLsa6 extends OspfLsa6 {
   uint8 rtrPriority;
   OspfOptions6 options;
   IPv6Address linkLocalAddress;
   OspfPrefix6 prefixes[];
}

class OspfIntraAreaPrefixLsa6 extends OspfLsa6 {
   uint16 referencedLsType enum(LsaType6) = 0;
   uint32 referencedLinkStateID;
   RouterID referencedAdvertisingRouter;
   OspfPrefix6 prefixes[];
}





// === OSPF packets ===========================================================
// http://tools.ietf.org/html/rfc5340#appendix-A.3

packet OspfPacket6 {
   uint8 version = 3;			// ospfv3
   uint8 type enum(OspfPacketType6) = HelloPacket;
   RouterID routerID;
   AreaID areaID;
   InstanceID instanceID = 0;	// by default							
}

// type: 1
packet OspfHelloPacket6 extends OspfPacket6 {	
   IfaceID interfaceID;
   uint8 rtrPriority;
   OspfOptions6 options;
   uint16 helloInterval;
   uint16 routerDeadInterval;
   DesignatedRouterID designatedRouterID;
   DesignatedRouterID backupDesignatedRouterID;
   RouterID neighborID[];
}

// type: 2
packet OspfDatabaseDescriptionPacket6 extends OspfPacket6 {
   uint16 interfaceMtu;
   OspfOptions6 options;
   OspfDdOptions6 ddOptions;
   uint32 ddSequenceNumber;
   OspfLsaHeader6 lsaHeaders[];
}

// type: 3
packet OspfLinkStateRequestPacket6 extends OspfPacket6 {
   OspfLsaRequest6 requests[];
}

// type: 4
packet OspfLinkStateUpdatePacket6 extends OspfPacket6 {
   uint32 numberOfLsas;
   OspfRouterLsa6 routerLsas[];
   OspfNetworkLsa6 networkLsas[];
   OspfInterAreaPrefixLsa6 interAreaPrefixLsas[];
   OspfInterAreaRouterLsa6 interAreaRouterLsas[];
   OspfAsExternalLsa6 asExternalLsas[];
   OspfLinkLsa6 linkLsas[];
   OspfIntraAreaPrefixLsa6 intraAreaPrefixLsas[];
}

// type: 5
packet OspfLinkStateAcknowledgementPacket6 extends OspfPacket6 {
   OspfLsaHeader6 lsaHeaders[];
}
	