//
// Copyright (C) 2009 - today, Brno University of Technology, Czech Republic
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
* @file CDPUpdate.cc
* @authors Tomas Rajca, Vladimir Vesely (ivesely@fit.vutbr.cz)
* @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
*/

#include "ansa/linklayer/cdp/CDPUpdate.h"

#include "inet/common/INETUtils.h"


namespace inet {

Register_Class(CDPUpdate);

#define TL_SIZE 4



std::ostream& operator<<(std::ostream& os, const CDPOptionODRDef& e)
{
    os << "blablabla";
    return os;
}

const TlvOptionBase *CDPUpdate::findOptionByType(short int optionType, int index) const
{
    int i = options.findByType(optionType, index);
    return i >= 0 ? getOption(i) : nullptr;
}

void CDPUpdate::addOption(TlvOptionBase *opt, int atPos)
{
    options.insertTlvOption(atPos, opt);
}

short CDPUpdate::getOptionLength(const TlvOptionBase *opt) const
{
    short length = 0;
    if(auto option = dynamic_cast<const CDPOptionPortId *>(opt))
    {
        length = strlen(option->getValue()) + TL_SIZE;
    }
    else if (auto option = dynamic_cast<const CDPOptionVersion *>(opt))
    {
        length = strlen(option->getValue()) + TL_SIZE;
    }
    else if (auto option = dynamic_cast<const CDPOptionPlatform *> (opt))
    {
        length = strlen(option->getValue()) + TL_SIZE;
    }
    else if (auto option = dynamic_cast<const CDPOptionVtp *> (opt))
    {
        length = strlen(option->getValue()) + TL_SIZE;
    }
    else if (auto option = dynamic_cast<const CDPOptionDevId *> (opt))
    {
        length = strlen(option->getValue()) + TL_SIZE;
    }
    else if (auto option = dynamic_cast<const CDPOptionCapa *> (opt))
    {
        length = option->getCapArraySize() + TL_SIZE;
    }
    else if (auto option = dynamic_cast<const CDPOptionDupl *> (opt))
    {
        length = sizeof(option->getFullDuplex()) + TL_SIZE;
    }
    else if (auto option = dynamic_cast<const CDPOptionAddr *> (opt))
    {
        short size = 0;
        addressType address;

        for(unsigned int i = 0; i < option->getAddressesArraySize(); ++i)
        {
            address = option->getAddresses(i);
            size += address.getProtocolArraySize() + strlen(address.getAddress());
            size += sizeof(address.getProtocolType()) + sizeof(address.getLength()) + sizeof(address.getAddressLen());
        }
        length = size + TL_SIZE;
    }
    else if (auto option = dynamic_cast<const CDPOptionPref *> (opt))
    {
        short size = 0;

        for(unsigned int i = 0; i < option->getPrefixesArraySize(); ++i)
            size += strlen(option->getPrefixes(i).getNetwork()) + 1;

        length = size + TL_SIZE;
    }
    else if (auto option = dynamic_cast<const CDPOptionODRDef *> (opt))
    {
        length = strlen(option->getDefaultRoute()) + TL_SIZE;
    }

    return length;
}


void CDPUpdate::setOptionLength(TlvOptionBase *opt)
{
    opt->setLength(getOptionLength(opt));
}

/**
 * Count the standard IP checksum for message.
 */
uint16_t CDPUpdate::countChecksum() const
{
    std::string a;
    const char *serialized;

    a += version;
    a += (char)ttl;
    for(unsigned int i=0; i < this->getOptionArraySize(); i++)
    {
        const TlvOptionBase *opt = this->getOption(i);
        if (auto option = dynamic_cast<const CDPOptionDevId *> (opt))
        {
            a += option->getType();
            a += option->getLength();
            a += option->getValue();
        }
        else if (auto option = dynamic_cast<const CDPOptionPortId *> (opt))
        {
            a += option->getType();
            a += option->getLength();
            a += option->getValue();
        }
        else if (auto option = dynamic_cast<const CDPOptionVersion *> (opt))
        {
            a += option->getType();
            a += option->getLength();
            a += option->getValue();
        }
        else if (auto option = dynamic_cast<const CDPOptionPlatform *> (opt))
        {
            a += option->getType();
            a += option->getLength();
            a += option->getValue();
        }
        else if (auto option = dynamic_cast<const CDPOptionVtp *> (opt))
        {
            a += option->getType();
            a += option->getLength();
            a += option->getValue();
        }
        else if (auto option = dynamic_cast<const CDPOptionCapa *> (opt))
        {
            a += option->getType();
            a += option->getLength();
            a += option->getCap(0);
            a += option->getCap(1);
            a += option->getCap(2);
            a += option->getCap(3);
        }
        else if (auto option = dynamic_cast<const CDPOptionDupl *> (opt))
        {
            a += option->getType();
            a += option->getLength();
            if(option->getFullDuplex())
                a += (char) 1;
            else
                a += (char) 0;
        }
        else if (auto option = dynamic_cast<const CDPOptionAddr *> (opt))
        {
            addressType address;

            a += option->getType();
            a += option->getLength();
            for(unsigned int i = 0; i < option->getAddressesArraySize(); ++i)
            {
                address = option->getAddresses(i);
                a += address.getProtocolType();
                a += address.getLength();
                for(unsigned int y = 0; y < address.getProtocolArraySize(); y++)
                    a += address.getProtocol(y);
                a += address.getAddressLen();
                a += address.getAddress();
            }

        }
        else if (auto option = dynamic_cast<const CDPOptionPref *> (opt))
        {
            prefixType prefix;

            a += option->getType();
            a += option->getLength();
            for(unsigned int i = 0; i < option->getPrefixesArraySize(); ++i)
            {
                prefix = option->getPrefixes(i);
                a += prefix.getNetwork();
                a += prefix.getMask();
            }
        }
        else if (auto option = dynamic_cast<const CDPOptionODRDef *> (opt))
        {
            a += option->getType();
            a += option->getLength();
            a += option->getDefaultRoute();
        }
    }
    serialized = a.c_str();
    int count = a.size();
    uint32_t sum = 0;

    while (count > 1) {
        sum += (serialized[0] << 8) | serialized[1];
        serialized += 2;
        if (sum & 0x80000000)
            sum = (sum & 0xFFFF) + (sum >> 16);
        count -= 2;
    }

    if (count)
        sum += *(const uint8_t *)serialized;

    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    return (uint16_t)sum;
}

} /* namespace inet */
