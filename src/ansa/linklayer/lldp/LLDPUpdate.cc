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

const TlvOptionBase *LLDPUpdate::findOptionByType(short int optionType, int index)
{
    int i = options.findByType(optionType, index);
    return i >= 0 ? getOption(i) : nullptr;
}

void LLDPUpdate::addOption(TlvOptionBase *opt, int atPos)
{
    options.insertTlvOption(atPos, opt);
}

short LLDPUpdate::getOptionLength(const TlvOptionBase *opt)
{
    short length = 0;
    if(dynamic_cast<const LLDPOptionEndOf *> (opt))
    {
        length = 1;         //can't be 0
    }
    else if (auto option = dynamic_cast<const LLDPOptionChassisId *> (opt))
    {
        length = strlen(option->getValue()) + sizeof(option->getSubtype());
    }
    else if (auto option = dynamic_cast<const LLDPOptionPortId *> (opt))
    {
        length = strlen(option->getValue()) + sizeof(option->getSubtype());
    }
    else if (auto option = dynamic_cast<const LLDPOptionTTL *> (opt))
    {
        length = sizeof(option->getTtl());
    }
    else if (auto option = dynamic_cast<const LLDPOptionPortDes *> (opt))
    {
        length = strlen(option->getValue());
    }
    else if (auto option = dynamic_cast<const LLDPOptionSystemName *> (opt))
    {
        length = strlen(option->getValue());
    }
    else if (auto option = dynamic_cast<const LLDPOptionSystemDes *> (opt))
    {
        length = strlen(option->getValue());
    }
    else if (auto option = dynamic_cast<const LLDPOptionCap *> (opt))
    {
        length = option->getEnCapArraySize() + option->getSysCapArraySize() + sizeof(option->getChasId());
    }
    else if (auto option = dynamic_cast<const LLDPOptionManAdd *> (opt))
    {
        length = sizeof(option->getAddLength()) + sizeof(option->getAddSubtype()) + strlen(option->getAddress());
        length += sizeof(option->getIfaceSubtype()) + sizeof(option->getIfaceNum());
        length += sizeof(option->getOidLength()) + strlen(option->getOid());
    }
    else if (auto option = dynamic_cast<const LLDPOptionOrgSpec *> (opt))
    {
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
    const LLDPOptionTTL *ttl = check_and_cast<const LLDPOptionTTL *> (getOption(2));

    return ttl->getTtl();
}

const char *LLDPUpdate::getChassisId()
{
    const LLDPOptionChassisId *chassisId = check_and_cast<const LLDPOptionChassisId *> (getOption(0));

    return chassisId->getValue();
}

const char *LLDPUpdate::getPortId()
{
    const LLDPOptionPortId *portId = check_and_cast<const LLDPOptionPortId *> (getOption(1));

    return portId->getValue();
}


void LLDPUpdate::setOptionLength(TlvOptionBase *opt)
{
    opt->setLength(getOptionLength(opt));
}


} /* namespace inet */
