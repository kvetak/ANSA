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

const char* ADDRESS_ATTR        = "address";
const char* IPV4_ATTR           = "ipv4";
const char* IPV6_ATTR           = "ipv6";
const char* ENABLED_VAL         = "enabled";
const int   NONE_SIMTIME        = -1;
const char* MAPSERVERADDR_TAG   = "MapServerAddress";
const char* MAPRESOLVERADDR_TAG = "MapResolverAddress";
const char* MAPSERVER_TAG       = "MapServer";
const char* MAPRESOLVER_TAG     = "MapResolver";
const char* ETRMAP_TAG          = "EtrMapping";
const short DATA_PORT           = 4341;
const short CONTROL_PORT        = 4342;
const char* SITE_TAG            = "Site";
const char* NAME_ATTR           = "name";
const char* KEY_ATTR            = "key";
const char* EID_TAG             = "EID";
const char* RLOC_TAG            = "RLOC";
const char* PRIORITY_ATTR       = "priority";
const char* WEIGHT_ATTR         = "weight";
const char* REGISTER_TIMER      = "LISP_REGISTER timer";
const char* REGISTER_MSG        = "LISP Map-Register";
const char* EMPTY_STRING_VAL    = "";
const char*         CONFIG_PAR              = "configData";
const char*         MS4_PAR                 = "mapServerV4";
const char*         MS6_PAR                 = "mapServerV6";
const char*         MR4_PAR                 = "mapResolverV4";
const char*         MR6_PAR                 = "mapResolverV6";
const char*         HASMAPDB_PAR            = "hasMapDB";
const char*         MAPCACHE_MOD            = "lispMapCache";
const char*         MAPDB_MOD               = "lispMapDatabase";
const unsigned char DEFAULT_EIDLENGTH_VAL   = 0;
const unsigned char DEFAULT_PRIORITY_VAL    = 1;
const unsigned char DEFAULT_WEIGHT_VAL      = 100;
const unsigned char DEFAULT_MPRIORITY_VAL    = 255;
const unsigned char DEFAULT_MWEIGHT_VAL      = 0;
const unsigned char DEFAULT_NONCE_VAL       = 0;
const unsigned short DEFAULT_TTL_VAL      = 1440;
const unsigned short DEFAULT_MAPVER_VAL      = 0;

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
