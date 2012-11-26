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

package inet.ansa.pim;

module pim
{
    parameters:
        // pim-mode
        @display("i=block/network2;bgl=4;bgb=255,207");
    gates:
        input networkLayerIn;
        output networkLayerOut;

    submodules:
        pimSplitter: pimSplitter {
            parameters:
                @display("p=85,95;q=queue;i=block/dispatch");
            gates:
                pimDMIn;
                pimSMIn;
                pimDMOut;
                pimSMOut;
                transportIn;
                transportOut;
        }

        pimDM: pimDM {
            parameters:
                @display("p=181,142;q=queue;i=block/network2");
            gates:
                splitterIn;
                spiltterOut;
        }

        pimSM: pimSM {
            parameters:
                @display("p=181,56;q=queue;i=block/network2");
            gates:
                splitterIn;
                spiltterOut;
        }

    connections:

        // pimDM
        pimSplitter.pimDMOut --> pimDM.splitterIn;
        pimSplitter.pimDMIn <-- pimDM.spiltterOut;

        // pimSM
        pimSplitter.pimSMOut --> pimSM.splitterIn;
        pimSplitter.pimSMIn <-- pimSM.spiltterOut;

        // splitter out
        pimSplitter.transportOut --> networkLayerOut;
        pimSplitter.transportIn <-- networkLayerIn;
}
