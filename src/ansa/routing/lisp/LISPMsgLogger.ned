//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
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
// @author Vladimir Vesely / ivesely@fit.vutbr.cz / http://www.fit.vutbr.cz/~ivesely/ 
// @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3

package ansa.routing.lisp;

simple LISPMsgLogger
{
    parameters:
    	@display("i=block/timer");
        @signal[sigLispSend](type=bool);
        @signal[sigLispRecv](type=bool);
        @signal[sigLispSizeSend](type=int);
        @signal[sigLispSizeRecv](type=int);
        @signal[sigLispMsg](type=int);
        @statistic[msg-send](title="msg send"; source=count(sigLispSend); record=last);
        @statistic[msg-recv](title="msg recieved"; source=count(sigLispRecv); record=last);
		@statistic[msg-size](title="msg size"; source=sum(sigLispSizeSend)+sum(sigLispSizeRecv); record=vector);
        @statistic[msg-sizesend](title="msg size send"; source=sum(sigLispSizeSend); record=last, vector?);
        @statistic[msg-sizerecv](title="msg size recieved"; source=sum(sigLispSizeRecv); record=last, vector?);
        @statistic[msg-type](title="msg type"; source=sigLispMsg; record=histogram; enum="UNKNOWN=0,REQUEST=1,REPLY=2,NEGREPLY=3,REGISTER=4,NOTIFY=5,ENCREQ=6,PROBEREQUEST=7,PROBEREPLY=8,CACHESYNC=9,CACHESYNCACK=10,DATA=11");
    	
}
