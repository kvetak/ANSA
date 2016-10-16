
cplusplus {{
#include "inet/linklayer/ethernet/Ethernet.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/linklayer/common/Ieee802Ctrl_m.h" // for enums
#include "inet/linklayer/ethernet/EtherFrame.h"
}}

namespace inet;

class noncobject MACAddress;

class noncobject MessageId;

enum EtherType;

enum SAPCode;

message EthernetIIFrame;


// ANSA ehter frame with VLAN

packet AnsaEtherFrame extends EthernetIIFrame
{
    int vlan;
    
    string displayString = "b=,,,kind,kind,0";
}

