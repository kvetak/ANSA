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
//
// @author Vladimir Vesely / ivesely@fit.vutbr.cz / http://www.fit.vutbr.cz/~ivesely/ 

cplusplus {{
    #include "INETDefs.h"
    #include "IPvXAddress.h"
    #include "LISPEidPrefix.h"
}}  

class noncobject IPvXAddress;
class noncobject LISPEidPrefix;

message LISPRegisterTimer {
    IPvXAddress serverAddress;
    unsigned short quickRegCycles;
}

message LISPRequestTimer {
    IPvXAddress srcEid;
    IPvXAddress dstEid;       
    unsigned long previousNonce; 
    unsigned char retryCount;
}

message LISPRlocProbeTimer {
    IPvXAddress rlocAddr;    
    LISPEidPrefix eidPref;
    unsigned long previousNonce;
    unsigned char retryCount;    
}

message LISPSyncTimer {
    IPvXAddress setMember;
    LISPEidPrefix eidPref;
    unsigned long previousNonce;
    unsigned char retryCount;      
}

message LISPMapEntryTimer {
    LISPEidPrefix eidPref;
}