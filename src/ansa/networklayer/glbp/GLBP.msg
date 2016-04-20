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

namespace inet;

enum STATE{
    DISABLED=1;
    INIT = 2;
    LISTEN = 4;
    SPEAK = 8;
    STANDBY = 16;
    ACTIVE = 32;
}

enum TYPE{
	HELLO = 1;
	REQRESP = 2;
	//AUTH = 3;
}

enum ADRTYPE{
	IPv4 = 1;
//	IPv6 = 2;
}