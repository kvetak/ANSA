
cplusplus {{
#include "Ethernet.h"
#include "MACAddress.h"
#include "Ieee802Ctrl_m.h" // for enums
#include "EtherFrame_m.h"
}}


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

