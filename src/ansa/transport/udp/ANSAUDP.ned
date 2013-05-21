//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

// @file ANSAUDP.ned
// @date 21.5.2013
// @author Jiri Trhlik (mailto:jiritm@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
// @brief @see ANSAUDP.h


package inet.ansa.transport.udp;

import inet.transport.udp.UDP;

//
// See ANSAUDP.h for more information
// @see ~UDP
//
simple ANSAUDP extends UDP
{	
	parameters:
	    @class(ANSAUDP);
	    //XXX: For some reason, when you open the UDP inspect window in the simulation
	    //and try to scroll down and you haven't defined a parameter, the simulation
	    //freezes
	    bool isANSAUDP = default(true);
}
