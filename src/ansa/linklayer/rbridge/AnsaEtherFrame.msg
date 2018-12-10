
import inet.common.packet.chunk.Chunk;
//import inet.linklayer.ethernet.Ethernet;
import inet.linklayer.common.MacAddress;
import inet.linklayer.common.Ieee802Ctrl;
import inet.linklayer.ethernet.EtherFrame;

cplusplus {{
//#include "inet/linklayer/ethernet/Ethernet.h"
//#include "inet/linklayer/common/MacAddress.h"
//#include "inet/linklayer/common/Ieee802Ctrl_m.h" // for enums
//#include "inet/linklayer/ethernet/EtherFrame.h"
}}

namespace inet;



//class noncobject MacAddress;

//class noncobject MessageId;
//class MessageId
//{
//    @existingClass;
//}

enum EtherType;

enum SapCode;

//message EthernetIIFrame;


// ANSA ehter frame with VLAN

class AnsaEtherFrame extends FieldsChunk
{
    int vlan;
    
    string displayString = "b=,,,kind,kind,0";
}

