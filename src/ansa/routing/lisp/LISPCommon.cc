//
// Copyright (C) 2013, 2014 Brno University of Technology
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
/**
 * @author Vladimir Vesely / ivesely@fit.vutbr.cz / http://www.fit.vutbr.cz/~ivesely/
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */

#include "ansa/routing/lisp/LISPCommon.h"
#include <bitset>
namespace inet {

// --------------- VALUES ---------------
const           char*   ENABLED_VAL                 = "enabled";
const           char*   EMPTY_STRING_VAL            = "";
const unsigned  char    DEFAULT_EIDLENGTH_VAL       = 0;
const unsigned  char    DEFAULT_PRIORITY_VAL        = 1;
const unsigned  char    DEFAULT_WEIGHT_VAL          = 100;
const unsigned  char    DEFAULT_MPRIORITY_VAL       = 255;
const unsigned  char    DEFAULT_MWEIGHT_VAL         = 0;
const unsigned  char    DEFAULT_NONCE_VAL           = 0;
const unsigned  short   DEFAULT_TTL_VAL             = 1440;
const unsigned  short   NOETR_TTL_VAL               = 1;
const unsigned  short   NOEID_TTL_VAL               = 15;
const unsigned  short   DEFAULT_MAPVER_VAL          = 0;
const           short   DATA_PORT_VAL               = 4341;
const           short   CONTROL_PORT_VAL            = 4342;
const unsigned  short   QUICK_REGTIMER_VAL          = 20;                       //20 seconds
const unsigned  short   QUICKREG_PERIOD_VAL         = 300;                      //5 minutes
const unsigned  short   QUICKREG_CYCLES_VAL         = QUICKREG_PERIOD_VAL / QUICK_REGTIMER_VAL + 1; //15+1 cycles
const unsigned  short   DEFAULT_REGTIMER_VAL        = 60;                       //60 seconds
const unsigned  short   DEFAULT_PROBETIMER_VAL      = 60;                       //60 seconds
const unsigned  short   DEFAULT_REQTIMEOUT_VAL      = 1;
const unsigned  short   DEFAULT_REQMULTIPLIER_VAL   = 2;
const unsigned  short   DEFAULT_MAXREQRETRIES_VAL   = 3;
const unsigned  short   DEFAULT_IPTTL_VAL           = 32;
const unsigned  short   DEFAULT_ETRTIMEOUT_VAL      = 180;
const           bool    PACKET_FORWARD              = true;
const           bool    PACKET_DROP                 = false;
const unsigned  short   LISPHDR_SIZE                = 8;


// --------------- TAGS/ATTRIBUTE ---------------
const           char*   ETRMAPSERVER_TAG        = "EtrMapServer";
const           char*   ITRMAPRESOLVER_TAG      = "ItrMapResolver";
const           char*   MAPSERVER_TAG           = "MapServer";
const           char*   MAPRESOLVER_TAG         = "MapResolver";
const           char*   ETRMAP_TAG              = "EtrMapping";
const           char*   SITE_TAG                = "Site";
const           char*   EID_TAG                 = "EID";
const           char*   RLOC_TAG                = "RLOC";
const           char*   SYNCSET_TAG             = "SynchronizationSet";
const           char*   SETMEMBER_TAG           = "SetMember";
const           char*   MAPCACHE_TAG            = "MapCache";

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
const           char*   CACHE_TIMER             = "LISP Cache Timer";
const           char*   CACHESYNC_TIMER         = "LISP CacheSync Timer";
const           char*   ETRTIMEOUT_TIMER        = "LISP ETRTimeout Timer";

const           char*   REGISTER_MSG            = "LISP Map-Register";
const           char*   NOTIFY_MSG              = "LISP Map-Notify";
const           char*   REQUEST_MSG             = "LISP Map-Request";
const           char*   NEGATIVEREPLY_MSG       = "LISP Negative Map-Reply";
const           char*   REPLY_MSG               = "LISP Map-Reply";
const           char*   ENCAPSULATEDREQUEST_MSG = "LISP Encapsulated Map-Request";
const           char*   REQUESTPROBE_MSG        = "LISP Map-Request RLOC-Probe";
const           char*   REPLYPROBE_MSG          = "LISP Map-Reply RLOC-Probe-Reply";
const           char*   CACHESYNC_MSG           = "LISP Cache-Sync";
const           char*   CACHESYNCACK_MSG        = "LISP Cache-Sync-Ack";
const           char*   DATA_MSG                = "LISP Data";

// --------------- GATES ---------------
const           char*   CONTROL_GATEIN          = "udpContrIn";
const           char*   CONTROL_GATEOUT         = "udpContrOut";
const           char*   DATA_GATEIN             = "udpDataIn";
const           char*   DATA_GATEOUT            = "udpDataOut";
const           char*   IPV4_GATEIN             = "ipv4In";
const           char*   IPV4_GATEOUT            = "ipv4Out";
const           char*   IPV6_GATEIN             = "ipv6In";
const           char*   IPV6_GATEOUT            = "ipv6Out";


// --------------- PARAMETERS ---------------
const           char*   CONFIG_PAR              = "configData";
const           char*   MS4_PAR                 = "mapServerV4";
const           char*   MS6_PAR                 = "mapServerV6";
const           char*   MR4_PAR                 = "mapResolverV4";
const           char*   MR6_PAR                 = "mapResolverV6";
const           char*   HASSITEDB_PAR           = "hasSiteDB";
const           char*   ACCEPTREQMAPPING_PAR    = "acceptMapRequestMapping";
const           char*   ECHONONCEALGO_PAR       = "echoNonceAlgo";
const           char*   CISCOSTARTUPDELAY_PAR   = "ciscoStartupDelays";
const           char*   MAPCACHETTL_PAR         = "mapCacheTtl";
const           char*   SMARTRLOCPROBE_PAR      = "smartRlocProbing";
const           char*   RLOCPROBINGALGO_PAR     = "rlocProbingAlgo";
const           char*   ALGO_CISCO_PARVAL       = "Cisco";
const           char*   ALGO_EIDRR_PARVAL       = "Simple";
const           char*   ALGO_EIDGROUPED_PARVAL  = "Sophisticated";
const           char*   CACHESYNC_PAR           = "cacheSynchronization";
const           char*   SYNCNONE_PARVAL         = "None";
const           char*   SYNCNAIVE_PARVAL        = "Naive";
const           char*   SYNCSMART_PARVAL        = "Smart";
const           char*   SYNCACK_PAR             = "cacheSyncAck";
const           char*   ADVERTONLYOWNEID_PAR    = "advertOnlyOwnEids";
const           char*   SSADDR_PAR              = "ssAddressType";
const           char*   SSADDR_NONLISP_PARVAL   = "nonLISP";
const           char*   SSADDR_RLOC_PARVAL      = "RLOC";
const           char*   SSADDR_EID_PARVAL       = "EID";

// --------------- MODULES ---------------
const           char*   MAPDB_MOD               = "^.lispMapDatabase";
const           char*   MAPCACHE_MOD            = "^.lispMapCache";
const           char*   SITEDB_MOD              = "^.lispSiteDatabase";
const           char*   LOGGER_MOD              = "^.lispMsgLogger";

// --------------- SIGNALS ---------------
const           char*   SIG_CACHE_LOOKUP        = "sigLispLookup";
const           char*   SIG_CACHE_MISS          = "sigLispMiss";
const           char*   SIG_CACHE_SIZE          = "sigLispSize";
const           char*   SIG_LOG_SEND            = "sigLispSend";
const           char*   SIG_LOG_RECV            = "sigLispRecv";
const           char*   SIG_LOG_MSG             = "sigLispMsg";
const           char*   SIG_PACKET_FORWARD      = "sigLispForward";
const           char*   SIG_PACKET_DROP         = "sigLispDrop";
const           char*   SIG_LOG_SIZESEND        = "sigLispSizeSend";
const           char*   SIG_LOG_SIZERECV        = "sigLispSizeRecv";




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
    const uint32 w1 = addr1.getInt();
    const uint32 w2 = addr2.getInt();
    //EV << "Addr1 - " << std::bitset<32>(w1).to_string() << " - " << addr1.str() << endl;
    //EV << "Addr2 - " << std::bitset<32>(w2).to_string() << " - " << addr2.str() << endl;
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
    //EV << "addr1 " << addr1 << "    addr2 " << addr2 << endl;
    uint32 *w1 = addr1.words();
    uint32 *w2 = addr2.words();

    /*
    EV << "Addr1 - "
            << std::bitset<32>(w1[0]).to_string() << " "
            << std::bitset<32>(w1[1]).to_string() << " "
            << std::bitset<32>(w1[2]).to_string() << " "
            << std::bitset<32>(w1[3]).to_string() << " - " << addr1.str() << endl;
    EV << "Addr2 - "
            << std::bitset<32>(w2[0]).to_string() << " "
            << std::bitset<32>(w2[1]).to_string() << " "
            << std::bitset<32>(w2[2]).to_string() << " "
            << std::bitset<32>(w2[3]).to_string() << " - " << addr2.str() << endl;
    */
    for (int j = 0; j <= 3; ++j)
    {
        uint32 res = w1[j] ^ w2[j];
        //EV << "kolo " << j << " ----- "<< w1[j] << " ---- " << w2[j] << endl;
        for (int i = 31; i >= 0; i--) {
            if (res & (1 << i)) {
               // 1, means not equal, so stop
               return 32 * j + 31 - i;
            }
        }
    }
    return -1;
}

/**
 *
 * @param addr1 First IPvX address for bit matching
 * @param addr2 Second IPvX address for bit matching
 * @return Returns either number of matching bits (N means number of matching bits from left),
 *         or -1 for exact match,
 *         or -2 when addresses are imcomparable (one is IPv4, another one IPv6).
 */
int LISPCommon::doPrefixMatch(L3Address addr1, L3Address addr2)
{
    if ( (addr1.getType() == L3Address::IPv6) xor (addr2.getType() == L3Address::IPv6) )
        return -2;
    int res;
    //IPv4
    if (! (addr1.getType() == L3Address::IPv6) )
        res = LISPCommon::getNumMatchingPrefixBits4(addr1.toIPv4(), addr2.toIPv4());
    //IPv6
    else
        res = LISPCommon::getNumMatchingPrefixBits6(addr1.toIPv6(), addr2.toIPv6());
    //EV << "Result " << res << endl;
    return res;
}

L3Address LISPCommon::getNetworkAddress(L3Address address, int length) {
    //IPv6
    if ( (address.getType() == L3Address::IPv6) && length <= 128 && length >= 0) {
        IPv6Address mask = IPv6Address::constructMask(length);
        uint32 *w1 = address.toIPv6().words();
        uint32 *w2 = mask.words();
        return IPv6Address(w1[0] & w2[0], w1[1] & w2[1], w1[2] & w2[2], w1[3] & w2[3]);
    }
    //IPv4
    else if (!(address.getType() == L3Address::IPv6) && length <= 32 && length >= 0) {
        IPv4Address mask = IPv4Address::makeNetmask(length);
        return address.toIPv4().doAnd(mask);
    }
    return L3Address();
}

}
