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

package ansa.routing.lisp;

simple LISPMapCache
{
    parameters:
        @display("i=block/table2");        
        xml 	configData 			 = default(xml("<MapCache></MapCache>"));        
        string 	cacheSynchronization = default("None"); //None OR Naive OR Smart
        bool   	cacheSyncAck 		 = default(false);   
        string  ssAddressType 		 = default("nonLISP"); //Either nonLISP OR RLOC OR EID
        
        @signal[sigLispLookup](type=bool);
        @signal[sigLispMiss](type=bool);
        @signal[sigLispSize](type=long);
        @statistic[cache-lookups](title="map-cache lookups"; source=count(sigLispLookup); record=last);
        @statistic[cache-misses](title="map-cache misses"; source=count(sigLispMiss); record=last);
        @statistic[cache-size](title="map-cache size"; source=sigLispSize; record=stats,vector);
}
