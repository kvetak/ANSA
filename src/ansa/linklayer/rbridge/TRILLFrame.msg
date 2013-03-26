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
// TODO generated message class
//
packet TRILLFrame {
    @customize(true);  // see the generated C++ header for more info
//    abstract int anotherField;
    uint16_t Ethertype; //this field is only present if IEEE 802.3 layer is used
    unsigned short version; //this field should be 2-bit unsigned int
    unsigned short reserved; //2b
    bool multiDest; //1b
    uint8_t opLength; //5b length of "options" filed in 4 octet units (up to 124 octets)
    uint8_t hopCount; //6b
    uint16_t egressRBNickname; // Egress RBridge Nickname: 16-bit identifier
    uint16_t ingressRBNickname; // Egress RBridge Nickname: 16-bit identifier
    uint32_t options[];
    
}
