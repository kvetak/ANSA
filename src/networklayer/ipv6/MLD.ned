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

package inet.networklayer.ipv6;
simple MLD like IMLD
{
    parameters:
        bool enabled = default(true);
        int    robustnessVariable = default(2); // IGMP is roboust to (roboustnessVariable-1) packet loss
        double queryInterval @unit("s") = default(125s);
        double queryResponseInterval @unit("s") = default(10s);
        double groupMembershipInterval @unit("s") = default((robustnessVariable * queryInterval) + queryResponseInterval);
        double otherQuerierPresentInterval @unit("s") = default((robustnessVariable * queryInterval) + (queryResponseInterval / 2));
        double startupQueryInterval @unit("s") = default(queryInterval / 4);
        int    startupQueryCount = default(robustnessVariable);
        double lastMemberQueryInterval @unit("s") = default(1s);
        int    lastMemberQueryCount = default(robustnessVariable);
        double unsolicitedReportInterval @unit("s") = default(10s);
//      double version1RouterPresentInterval @unit("s") = default(400s);
        @display("i=block/cogwheel_s");

    gates:
        input ipIn @labels(IPv6ControlInfo/up);  // delivered IGMP packets
        output ipOut @labels(IPv6ControlInfo/down); // to ~IPv4

        input routerIn @labels(IPv6ControlInfo/up);
        output routerOut @labels(IPv6ControlInfo/down);
}