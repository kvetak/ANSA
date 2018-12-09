
cplusplus {{
#include "inet/linklayer/ethernet/Ethernet.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/linklayer/common/Ieee802Ctrl_m.h" // for enums
#include "inet/linklayer/ethernet/EtherFrame.h"
}}

namespace inet;

class noncobject MacAddress;

class noncobject MessageId;

enum EtherType;

enum SapCode;

message EthernetIIFrame;


// ANSA ehter frame with VLAN

packet AnsaEtherFrame extends EthernetIIFrame
{
    int vlan;
    
    string displayString = "b=,,,kind,kind,0";
}

