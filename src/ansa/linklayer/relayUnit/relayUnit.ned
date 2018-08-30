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

package ansa.linklayer.relayUnit;

import ansa.linklayer.contract.ANSA_IMACRelayUnit;


//
// This modul forwards frames (~EtherFrame) based on their destination MAC addresses to appropriate ports.
//
// It can work in two mode:
// - Stp unaware mode (If no module connected to its Stp gate):
//     Looks up the frames destination address in the ~MacAddressTable, if it found the address, sends through
//	   that port, or otherwise broadcasts it.
// - Stp aware mode (If an ~Stp or ~Rstp module connected to its Stp gate):
//     Handles Bpdu frames.
//     If relaying a message also checks the port's status in the InterfaceTable.
//
// @see EtherFrame, MacAddressTable, Stp, Rstp
//

//from Ieee8021dRelay
simple relayUnit like ANSA_IMACRelayUnit
{
    parameters:
        @display("i=block/broadcast;bgb=425,114");
        string interfaceTablePath = default("^.interfaceTable"); // The path to the InterfaceTable module
        string macTablePath = default("^.macTable");             // The path to the MacAddressTable module
    gates:
        input ifIn[];
        output ifOut[];
        input cdpIn @loose;
        output cdpOut @loose;
        input lldpIn @loose;
        output lldpOut @loose;
}
