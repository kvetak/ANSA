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

#ifndef ANSAETHERMACBASEVF_H_
#define ANSAETHERMACBASEVF_H_

#include "EtherMACBase.h"

// Forward declarations:
class EtherFrame;
class EtherTraffic;
class InterfaceEntry;
class AnsaInterfaceEntry;

class AnsaEtherMACBaseVF: public EtherMACBase {
public:
    AnsaEtherMACBaseVF() {};
    virtual ~AnsaEtherMACBaseVF() {};

    protected:
        virtual void initialize();
        virtual void registerInterface();
        virtual bool dropFrameNotForUs(EtherFrame *frame);

};

#endif /* ANSAETHERMACBASEVF_H_ */
