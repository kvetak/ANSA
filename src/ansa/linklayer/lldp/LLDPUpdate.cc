//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
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
* @file LLDPUpdate.cc
* @author Tomas Rajca
* @author Vladimir Vesely (ivesely@fit.vutbr.cz)
* @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
*/

#include "ansa/linklayer/lldp/LLDPUpdate.h"

#include "inet/common/INETUtils.h"


namespace inet {

Register_Class(LLDPUpdate);

TlvOptionBase *LLDPUpdate::findOptionByType(short int optionType, int index)
{
    int i = options.findByType(optionType, index);
    return i >= 0 ? &getOption(i) : nullptr;
}

void LLDPUpdate::addOption(TlvOptionBase *opt, int atPos)
{
    options.add(opt, atPos);
}

short LLDPUpdate::getOptionLength(TlvOptionBase *opt)
{
    short length = 0;
    if(dynamic_cast<LLDPOptionEndOf *> (opt))
    {
        length = 1;         //can't be 0
    }
    else if(dynamic_cast<LLDPOptionChassisId *> (opt))
    {
        LLDPOptionChassisId *option = dynamic_cast<LLDPOptionChassisId *> (opt);
        length = strlen(option->getValue()) + sizeof(option->getSubtype());
    }
    else if(dynamic_cast<LLDPOptionPortId *> (opt))
    {
        LLDPOptionPortId *option = dynamic_cast<LLDPOptionPortId *> (opt);
        length = strlen(option->getValue()) + sizeof(option->getSubtype());
    }
    else if(dynamic_cast<LLDPOptionTTL *> (opt))
    {
        LLDPOptionTTL *option = dynamic_cast<LLDPOptionTTL *> (opt);
        length = sizeof(option->getTtl());
    }
    else if(dynamic_cast<LLDPOptionPortDes *> (opt))
    {
        LLDPOptionPortDes *option = dynamic_cast<LLDPOptionPortDes *> (opt);
        length = strlen(option->getValue());
    }
    else if(dynamic_cast<LLDPOptionSystemName *> (opt))
    {
        LLDPOptionSystemName *option = dynamic_cast<LLDPOptionSystemName *> (opt);
        length = strlen(option->getValue());
    }
    else if(dynamic_cast<LLDPOptionSystemDes *> (opt))
    {
        LLDPOptionSystemDes *option = dynamic_cast<LLDPOptionSystemDes *> (opt);
        length = strlen(option->getValue());
    }
    else if(dynamic_cast<LLDPOptionCap *> (opt))
    {
        LLDPOptionCap *option = dynamic_cast<LLDPOptionCap *> (opt);
        length = option->getEnCapArraySize() + option->getSysCapArraySize() + sizeof(option->getChasId());
    }
    else if(dynamic_cast<LLDPOptionManAdd *> (opt))
    {
        LLDPOptionManAdd *option = dynamic_cast<LLDPOptionManAdd *> (opt);
        length = sizeof(option->getAddLength()) + sizeof(option->getAddSubtype()) + strlen(option->getAddress());
        length += sizeof(option->getIfaceSubtype()) + sizeof(option->getIfaceNum());
        length += sizeof(option->getOidLength()) + strlen(option->getOid());
    }
    else if(dynamic_cast<LLDPOptionOrgSpec *> (opt))
    {
        LLDPOptionOrgSpec *option = dynamic_cast<LLDPOptionOrgSpec *> (opt);
        length = sizeof(option->getOui()) + sizeof(option->getSubtype()) + strlen(option->getValue());
    }

    return length;
}

std::string LLDPUpdate::getMsap()
{
    std::string s;

    // MSAP get from contatenation Chassis ID and Port ID
    s = getChassisId();
    s += getPortId();

    return s;
}

uint16_t LLDPUpdate::getTtl()
{
    LLDPOptionTTL *ttl = dynamic_cast<LLDPOptionTTL *> (&getOption(2));

    return ttl->getTtl();
}

const char *LLDPUpdate::getChassisId()
{
    LLDPOptionChassisId *chassisId = dynamic_cast<LLDPOptionChassisId *> (&getOption(0));

    return chassisId->getValue();
}

const char *LLDPUpdate::getPortId()
{
    LLDPOptionPortId *portId = dynamic_cast<LLDPOptionPortId *> (&getOption(1));

    return portId->getValue();
}


void LLDPUpdate::setOptionLength(TlvOptionBase *opt)
{
    opt->setLength(getOptionLength(opt));
}


} /* namespace inet */
