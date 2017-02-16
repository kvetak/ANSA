//
// Copyright (C) 2009 - today Brno University of Technology
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
//
// @author Vladimir Vesely / ivesely@fit.vutbr.cz / http://www.fit.vutbr.cz/~ivesely/
// @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3

cplusplus {{
    #include "inet/common/INETDefs.h"
    #include "inet/networklayer/common/L3Address.h"
    #include "ansa/routing/lisp/LISPEidPrefix.h"  
}}

namespace inet;  

class noncobject L3Address;
class noncobject LISPEidPrefix;

message LISPRegisterTimer {
    L3Address serverAddress;
    unsigned short quickRegCycles;
}

message LISPRequestTimer {
    L3Address srcEid;
    L3Address dstEid;       
    unsigned long previousNonce; 
    unsigned char retryCount;
}

message LISPRlocProbeTimer {
    L3Address rlocAddr;    
    LISPEidPrefix eidPref;
    unsigned long previousNonce;
    unsigned char retryCount;    
}

message LISPSyncTimer {
    L3Address setMember;
    LISPEidPrefix eidPref;
    unsigned long previousNonce;
    unsigned char retryCount;      
}

message LISPMapEntryTimer {
    LISPEidPrefix eidPref;
}

message LISPEtrTimer {
    string siteName;
    L3Address etrAddr;
}
  
