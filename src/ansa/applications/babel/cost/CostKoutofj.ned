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

// Author Vit Rek (mailto:xrekvi00@stud.fit.vutbr.cz)
// Link cost computation module uses k-out-of-j method

package inet.ansa.applications.babel.cost;

simple CostKoutofj
{
        parameters:
        int k = default(2);
        int j = default(3);
        
        @display("i=block/rxtx");
}
