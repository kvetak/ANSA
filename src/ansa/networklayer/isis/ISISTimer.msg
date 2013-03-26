/// Author: Matej Hrncirik
// FIT VUT 2012
//
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


enum ISISTimerType
{
   
   HELLO_TIMER = 1;
   NEIGHBOUR_DEAD = 2; //hello timer expired
   LSP_REFRESH = 3;
   LSP_DEAD = 4;
   CSNP_TIMER = 5;
   LSP_DELETE = 6;
   //L2LSP_DEAD = 7;
   PERIODIC_SEND = 8;
   //LSP_REFRESH = 9;
   PSNP_TIMER = 10;
   GENERATE_LSP_TIMER = 11;
   ISIS_START = 12;
   SPF_FULL = 13; //fortunately it's not friday
   TRILL_HELLO_TIMER = 14;
   
   
};


message ISISTimer extends cMessage
{
      char 		timerKind enum(ISISTimerType);
      int		interfaceIndex; //index to ISISIft maybe change to gateway index CHANGED TO gateIndex
      int		gateIndex; //network layer gate eg. physical interface
      unsigned char	sysID[6];	//for specific timers eg. neighbour timeout
      unsigned short isType;	//L1 / L2
      unsigned char areaID[3];	//areaID; relevant for L2 dead timer
      unsigned char LSPid[8];	//for use as LSP dead timer
      
};