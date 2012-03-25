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

#ifndef ANSAOSPFSTAT6_H_
#define ANSAOSPFSTAT6_H_

namespace AnsaOspf6 {

class Stat {
private:
    unsigned long helloPacketSend;
    unsigned long helloPacketReceived;
    unsigned long ospfPacketSend;
    unsigned long ospfPacketReceived;

public:
    Stat();
    void AddHelloPacketSend()                {++helloPacketSend;}
    void AddHelloPacketReceived()            {++helloPacketReceived;}
    void AddOspfPacketSend()                 {++ospfPacketSend;}
    void AddOspfPacketReceived()             {++ospfPacketReceived;}
    unsigned long GetHelloPacketSend()       {return helloPacketSend;}
    unsigned long GetHelloPacketReceived()   {return helloPacketReceived;}
    unsigned long GetOspfPacketSend()        {return ospfPacketSend;}
    unsigned long GetOspfPacketReceived()    {return ospfPacketReceived;}
};

}

#endif /* ANSAOSPFSTAT6_H_ */
