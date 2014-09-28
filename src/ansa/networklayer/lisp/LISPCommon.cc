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

#include <LISPCommon.h>

// --------------- VALUES ---------------
const           char*   ENABLED_VAL             = "enabled";
const           char*   EMPTY_STRING_VAL        = "";
const unsigned  char    DEFAULT_EIDLENGTH_VAL   = 0;
const unsigned  char    DEFAULT_PRIORITY_VAL    = 1;
const unsigned  char    DEFAULT_WEIGHT_VAL      = 100;
const unsigned  char    DEFAULT_MPRIORITY_VAL   = 255;
const unsigned  char    DEFAULT_MWEIGHT_VAL     = 0;
const unsigned  char    DEFAULT_NONCE_VAL       = 0;
const unsigned  short   DEFAULT_TTL_VAL         = 1440;
const unsigned  short   NOETR_TTL_VAL           = 1;
const unsigned  short   NOEID_TTL_VAL           = 15;
const unsigned  short   DEFAULT_MAPVER_VAL      = 0;
const           short   DATA_PORT_VAL           = 4341;
const           short   CONTROL_PORT_VAL        = 4342;
const unsigned  short   QUICK_REGTIMER_VAL      = 20;                       //20 seconds
const unsigned  short   QUICKREG_PERIOD_VAL     = 300;                      //5 minutes
const unsigned  short   QUICKREG_CYCLES_VAL     = QUICKREG_PERIOD_VAL / QUICK_REGTIMER_VAL + 1; //15+1 cycles
const unsigned  short   DEFAULT_REGTIMER_VAL    = 60;                       //60 seconds
const unsigned short    DEFAULT_REQTIMEOUT_VAL  = 1;
const unsigned short    DEFAULT_REQMULTIPLIER_VAL= 2;
const unsigned short    DEFAULT_MAXREQRETRIES_VAL= 3;
const unsigned short    DEFAULT_IPTTL_VAL       = 32;

// --------------- TAGS/ATTRIBUTE ---------------
const           char*   ETRMAPSERVER_TAG        = "EtrMapServer";
const           char*   ITRMAPRESOLVER_TAG      = "ItrMapResolver";
const           char*   MAPSERVER_TAG           = "MapServer";
const           char*   MAPRESOLVER_TAG         = "MapResolver";
const           char*   ETRMAP_TAG              = "EtrMapping";
const           char*   SITE_TAG                = "Site";
const           char*   EID_TAG                 = "EID";
const           char*   RLOC_TAG                = "RLOC";

const           char*   ADDRESS_ATTR            = "address";
const           char*   IPV4_ATTR               = "ipv4";
const           char*   IPV6_ATTR               = "ipv6";
const           char*   NAME_ATTR               = "name";
const           char*   KEY_ATTR                = "key";
const           char*   PRIORITY_ATTR           = "priority";
const           char*   WEIGHT_ATTR             = "weight";
const           char*   PROXY_ATTR              = "proxy";
const           char*   NOTIFY_ATTR             = "want-map-notify";
const           char*   QUICKREG_ATTR           = "quick-registration";
const           char*   LOCAL_ATTR              = "local";

// --------------- TIMERS/MESSAGES ---------------
const           char*   REGISTER_TIMER          = "LISP Register Timer";
const           char*   REQUEST_TIMER           = "LISP Request Timer";
const           char*   REQUESTPROBE_TIMER      = "LISP RLOC-Probe Timer";
const           char*   REGISTER_MSG            = "LISP Map-Register";
const           char*   NOTIFY_MSG              = "LISP Map-Notify";
const           char*   REQUEST_MSG             = "LISP Map-Request";
const           char*   NEGATIVEREPLY_MSG       = "LISP Negative Map-Reply";
const           char*   REPLY_MSG               = "LISP Map-Reply";
const           char*   ENCAPSULATEDREQUEST_MSG = "LISP Encapsulated Map-Request";
const           char*   REQUESTPROBE_MSG        = "LISP Map-Request RLOC-Probe";
const           char*   REPLYPROBE_MSG          = "LISP Map-Reply RLOC-Probe-Reply";

// --------------- PARAMETERS ---------------
const           char*   CONFIG_PAR              = "configData";
const           char*   MS4_PAR                 = "mapServerV4";
const           char*   MS6_PAR                 = "mapServerV6";
const           char*   MR4_PAR                 = "mapResolverV4";
const           char*   MR6_PAR                 = "mapResolverV6";
const           char*   HASMAPDB_PAR            = "hasMapDB";
const           char*   ACCEPTREQMAPPING_PAR    = "acceptMapRequestMapping";
const           char*   MAPCACHETTL_PAR         = "mapCacheTtl";
const           char*   SMARTRLOCPROBE_PAR      = "smartRlocProbing";
const           char*   RLOCPROBINGALGO_PAR     = "rlocProbingAlgo";
const           char*   ALGO_CISCO_PARVAL       = "Cisco";
const           char*   ALGO_EIDRR_PARVAL       = "EidRoundRobin";
const           char*   ALGO_EIDGROUPED_PARVAL  = "EidGrouped";

// --------------- MODULES ---------------
const           char*   MAPCACHE_MOD            = "lispMapCache";
const           char*   MAPDB_MOD               = "lispMapDatabase";


LISPCommon::LISPCommon() {
    // TODO Auto-generated constructor stub

}

LISPCommon::~LISPCommon() {
    // TODO Auto-generated destructor stub

}

void LISPCommon::parseIpAddress(const char* str, std::string& address, std::string& length) {
    std::string addr = str;
    int pos = addr.find("/");
    std::string len = addr.substr(pos + 1);
    addr.replace(pos, -1, "");

    address = addr;
    length = len;
}

int LISPCommon::getNumMatchingPrefixBits4(IPv4Address addr1, IPv4Address addr2)
{
    uint32 w1 = addr1.getInt();
    uint32 w2 = addr2.getInt();
    uint32 res = w1 ^ w2;
    // If the bits are equal, there is a 0, so counting
    // the zeros from the left
    for (int i = 31; i >= 0; i--) {
        if (res & (1 << i)) {
            // 1, means not equal, so stop
            return 31 - i;
        }
    }
    return -1;
}


int LISPCommon::getNumMatchingPrefixBits6(IPv6Address addr1, IPv6Address addr2)
{
    for (int j = 3; j != 0; j--)
    {
        const uint32 *w1 = addr1.words();
        const uint32 *w2 = addr2.words();
        uint32 res = w1[j] ^ w2[j];
        for (int i = 31; i >= 0; i--) {
            if (res & (1 << i)) {
               // 1, means not equal, so stop
               return 32 * j + 31 - i;
            }
        }
    }
    return -1;
}

int LISPCommon::doPrefixMatch(IPvXAddress addr1, IPvXAddress addr2)
{
    if ( addr1.isIPv6() xor addr2.isIPv6() )
        return -2;
    //IPv4
    if (!addr1.isIPv6())
        return LISPCommon::getNumMatchingPrefixBits4(addr1.get4(), addr2.get4());
    //IPv6
    return LISPCommon::getNumMatchingPrefixBits6(addr1.get6(), addr2.get6());
}

