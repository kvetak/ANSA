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
/**
* @author Jan Holusa
* @author Vladimir Vesely (ivesely@fit.vutbr.cz)
* @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
*/

#include "ansa/routing/glbp/GLBPMessage.h"

#include "inet/common/INETUtils.h"

namespace inet {

Register_Class(GLBPMessage);

TlvOptionBase *GLBPMessage::findOptionByType(short int optionType, int index)
{
    int i = getTLV().findByType(optionType, index);
    return i >= 0 ? &getTlvOption(i) : nullptr;
}

void GLBPMessage::addOption(TlvOptionBase *opt, int atPos)
{
    getTLV().add(opt, atPos);
}

} /* namespace inet */
