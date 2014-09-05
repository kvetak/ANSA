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

#ifndef LISPCOMMON_H_
#define LISPCOMMON_H_

#include <string>

extern const char* ENABLED_VAL;
extern const char* EMPTY_STRING_VAL;
extern const unsigned char DEFAULT_EIDLENGTH_VAL;
extern const unsigned char DEFAULT_PRIORITY_VAL;
extern const unsigned char DEFAULT_WEIGHT_VAL;
extern const unsigned char DEFAULT_MPRIORITY_VAL;
extern const unsigned char DEFAULT_MWEIGHT_VAL;
extern const unsigned char DEFAULT_NONCE_VAL;
extern const unsigned short DEFAULT_TTL_VAL;
extern const unsigned short DEFAULT_MAPVER_VAL;

extern const int   NONE_SIMTIME;

extern const short DATA_PORT;
extern const short CONTROL_PORT;

extern const char* SITE_TAG;
extern const char* EID_TAG;
extern const char* RLOC_TAG;
extern const char* MAPSERVER_TAG;
extern const char* MAPRESOLVER_TAG;
extern const char* MAPSERVERADDR_TAG;
extern const char* MAPRESOLVERADDR_TAG;
extern const char* ETRMAP_TAG;

extern const char* ADDRESS_ATTR;
extern const char* IPV4_ATTR;
extern const char* IPV6_ATTR;
extern const char* NAME_ATTR;
extern const char* KEY_ATTR;
extern const char* PRIORITY_ATTR;
extern const char* WEIGHT_ATTR;

extern const char* REGISTER_TIMER;

extern const char* REGISTER_MSG;

extern const char* CONFIG_PAR;
extern const char* MS4_PAR;
extern const char* MS6_PAR;
extern const char* MR4_PAR;
extern const char* MR6_PAR;
extern const char* HASMAPDB_PAR;

extern const char* MAPCACHE_MOD;
extern const char* MAPDB_MOD;

class LISPCommon {
  public:
    LISPCommon();
    virtual ~LISPCommon();

    enum EKeyIds
    {
        NONE                = 0,
        HMAC_SHA_1_96       = 1,
        HMAC_SHA_256_128    = 2,
    };

    enum EAct {
        NO_ACTION           = 0,
        NATIVELY_FORWARD    = 1,
        SEND_MAP_REQUEST    = 2,
        DROP                = 3,
    };

    static void parseIpAddress(const char* str, std::string &address, std::string &length);
};

#endif /* LISPCOMMON_H_ */
