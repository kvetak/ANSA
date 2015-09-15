
cplusplus {{
#include "inet/linklayer/ethernet/Ethernet.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/linklayer/common/Ieee802Ctrl_m.h" // for enums
#include "inet/linklayer/ethernet/EtherFrame_m.h"
}}


class noncobject inet::MACAddress;

class noncobject inet::MessageId;

enum EtherType;

enum SAPCode;

message inet::EthernetIIFrame;


// ANSA ehter frame with VLAN

packet AnsaEtherFrame extends EthernetIIFrame
{
    int vlan;
    
    string displayString = "b=,,,kind,kind,0";
}

