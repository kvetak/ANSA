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

#ifndef LISPCOMMON_H_
#define LISPCOMMON_H_

#include <string>
#include "inet/networklayer/common/L3Address.h"
namespace inet {
extern const char* ENABLED_VAL;
extern const char* EMPTY_STRING_VAL;
extern const unsigned char DEFAULT_EIDLENGTH_VAL;
extern const unsigned char DEFAULT_PRIORITY_VAL;
extern const unsigned char DEFAULT_WEIGHT_VAL;
extern const unsigned char DEFAULT_MPRIORITY_VAL;
extern const unsigned char DEFAULT_MWEIGHT_VAL;
extern const unsigned char DEFAULT_NONCE_VAL;
extern const unsigned short DEFAULT_TTL_VAL;
extern const unsigned short NOETR_TTL_VAL;
extern const unsigned short NOEID_TTL_VAL;
extern const unsigned short DEFAULT_MAPVER_VAL;
extern const short DATA_PORT_VAL;
extern const short CONTROL_PORT_VAL;
extern const unsigned short QUICK_REGTIMER_VAL;
extern const unsigned short QUICKREG_PERIOD_VAL;
extern const unsigned short QUICKREG_CYCLES_VAL;
extern const unsigned short DEFAULT_REGTIMER_VAL;
extern const unsigned short DEFAULT_PROBETIMER_VAL;
extern const unsigned short DEFAULT_REQTIMEOUT_VAL;
extern const unsigned short DEFAULT_REQMULTIPLIER_VAL;
extern const unsigned short DEFAULT_MAXREQRETRIES_VAL;
extern const unsigned short DEFAULT_IPTTL_VAL;
extern const unsigned short DEFAULT_ETRTIMEOUT_VAL;
extern const bool PACKET_FORWARD;
extern const bool PACKET_DROP;
extern const unsigned short LISPHDR_SIZE;

extern const char* SITE_TAG;
extern const char* EID_TAG;
extern const char* RLOC_TAG;
extern const char* MAPSERVER_TAG;
extern const char* MAPRESOLVER_TAG;
extern const char* ETRMAPSERVER_TAG;
extern const char* ITRMAPRESOLVER_TAG;
extern const char* ETRMAP_TAG;
extern const char* SYNCSET_TAG;
extern const char* SETMEMBER_TAG;
extern const char* MAPCACHE_TAG;
extern const char* ROUTING_TAG;
extern const char* LISP_TAG;

extern const char* ADDRESS_ATTR;
extern const char* IPV4_ATTR;
extern const char* IPV6_ATTR;
extern const char* NAME_ATTR;
extern const char* KEY_ATTR;
extern const char* PRIORITY_ATTR;
extern const char* WEIGHT_ATTR;
extern const char* PROXY_ATTR;
extern const char* NOTIFY_ATTR;
extern const char* QUICKREG_ATTR;
extern const char* LOCAL_ATTR;

extern const char* REGISTER_TIMER;
extern const char* REQUEST_TIMER;
extern const char* REQUESTPROBE_TIMER;
extern const char* CACHE_TIMER;
extern const char* CACHESYNC_TIMER;
extern const char* ETRTIMEOUT_TIMER;

extern const char* REGISTER_MSG;
extern const char* NOTIFY_MSG;
extern const char* REQUEST_MSG;
extern const char* NEGATIVEREPLY_MSG;
extern const char* REPLY_MSG;
extern const char* ENCAPSULATEDREQUEST_MSG;
extern const char* REQUESTPROBE_MSG;
extern const char* REPLYPROBE_MSG;
extern const char* CACHESYNC_MSG;
extern const char* CACHESYNCACK_MSG;
extern const char* DATA_MSG;

extern const char* CONTROL_GATEIN;
extern const char* CONTROL_GATEOUT;
extern const char* DATA_GATEIN;
extern const char* DATA_GATEOUT;
extern const char* IPV4_GATEIN;
extern const char* IPV4_GATEOUT;
extern const char* IPV6_GATEIN;
extern const char* IPV6_GATEOUT;

extern const char* CONFIG_PAR;
extern const char* MS4_PAR;
extern const char* MS6_PAR;
extern const char* MR4_PAR;
extern const char* MR6_PAR;
extern const char* HASSITEDB_PAR;
extern const char* ACCEPTREQMAPPING_PAR;
extern const char* ECHONONCEALGO_PAR;
extern const char* CISCOSTARTUPDELAY_PAR;
extern const char* MAPCACHETTL_PAR;
extern const char* SMARTRLOCPROBE_PAR;
extern const char* RLOCPROBINGALGO_PAR;
extern const char* ALGO_CISCO_PARVAL;
extern const char* ALGO_EIDRR_PARVAL;
extern const char* ALGO_EIDGROUPED_PARVAL;
extern const char* CACHESYNC_PAR;
extern const char* SYNCNONE_PARVAL;
extern const char* SYNCNAIVE_PARVAL;
extern const char* SYNCSMART_PARVAL;
extern const char* SYNCACK_PAR;
extern const char* ADVERTONLYOWNEID_PAR;
extern const char* SSADDR_PAR;
extern const char* SSADDR_NONLISP_PARVAL;
extern const char* SSADDR_RLOC_PARVAL;
extern const char* SSADDR_EID_PARVAL;

extern const char* MAPDB_MOD;
extern const char* MAPCACHE_MOD;
extern const char* SITEDB_MOD;
extern const char* LOGGER_MOD;

extern const char* SIG_CACHE_LOOKUP;
extern const char* SIG_CACHE_MISS;
extern const char* SIG_CACHE_SIZE;
extern const char* SIG_LOG_SEND;
extern const char* SIG_LOG_RECV;
extern const char* SIG_LOG_MSG;
extern const char* SIG_PACKET_FORWARD;
extern const char* SIG_PACKET_DROP;
extern const char* SIG_LOG_SIZESEND;
extern const char* SIG_LOG_SIZERECV;



class LISPCommon {
  public:
    enum EKeyIds
    {
        KID_NONE                = 0,
        KID_HMAC_SHA_1_96       = 1,
        KID_HMAC_SHA_256_128    = 2,
    };

    enum EAct {
        NO_ACTION           = 0,
        NATIVELY_FORWARD    = 1,
        SEND_MAP_REQUEST    = 2,
        DROP                = 3,
    };

    enum Afi {AFI_UNKNOWN = 0, AFI_IPV4 = 1, AFI_IPV6 = 2};

    static void parseIpAddress(const char* str, std::string &address, std::string &length);

    static int doPrefixMatch(L3Address addr1, L3Address addr2);
    static int getNumMatchingPrefixBits4(Ipv4Address addr1, Ipv4Address addr2);
    static int getNumMatchingPrefixBits6(Ipv6Address addr1, Ipv6Address addr2);

    static L3Address getNetworkAddress(L3Address address, int length);

};
}
#endif /* LISPCOMMON_H_ */
