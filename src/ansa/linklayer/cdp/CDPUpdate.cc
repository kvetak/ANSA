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
* @author Tomas Rajca
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

TLVOptionBase *CDPUpdate::findOptionByType(short int optionType, int index)
{
    int i = options.findByType(optionType, index);
    return i >= 0 ? &getOption(i) : nullptr;
}

void CDPUpdate::addOption(TLVOptionBase *opt, int atPos)
{
    options.add(opt, atPos);
}

short CDPUpdate::getOptionLength(TLVOptionBase *opt)
{
    short length = 0;
    if(dynamic_cast<CDPOptionPortId *> (opt))
    {
        CDPOptionPortId *option = dynamic_cast<CDPOptionPortId *> (opt);
        length = strlen(option->getValue()) + TL_SIZE;
    }
    else if(dynamic_cast<CDPOptionVersion *> (opt))
    {
        CDPOptionVersion *option = dynamic_cast<CDPOptionVersion *> (opt);
        length = strlen(option->getValue()) + TL_SIZE;
    }
    else if(dynamic_cast<CDPOptionPlatform *> (opt))
    {
        CDPOptionPlatform *option = dynamic_cast<CDPOptionPlatform *> (opt);
        length = strlen(option->getValue()) + TL_SIZE;
    }
    else if(dynamic_cast<CDPOptionVtp *> (opt))
    {
        CDPOptionVtp *option = dynamic_cast<CDPOptionVtp *> (opt);
        length = strlen(option->getValue()) + TL_SIZE;
    }
    else if(dynamic_cast<CDPOptionDevId *> (opt))
    {
        CDPOptionDevId *option = dynamic_cast<CDPOptionDevId *> (opt);
        length = strlen(option->getValue()) + TL_SIZE;
    }
    else if(dynamic_cast<CDPOptionCapa *> (opt))
    {
        CDPOptionCapa *option = dynamic_cast<CDPOptionCapa *> (opt);
        length = option->getCapArraySize() + TL_SIZE;
    }
    else if(dynamic_cast<CDPOptionDupl *> (opt))
    {
        CDPOptionDupl *option = dynamic_cast<CDPOptionDupl *> (opt);
        length = sizeof(option->getFullDuplex()) + TL_SIZE;
    }
    else if(dynamic_cast<CDPOptionAddr *> (opt))
    {
        CDPOptionAddr *option = dynamic_cast<CDPOptionAddr *> (opt);
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
    else if(dynamic_cast<CDPOptionPref *> (opt))
    {
        CDPOptionPref *option = dynamic_cast<CDPOptionPref *> (opt);
        short size = 0;

        for(unsigned int i = 0; i < option->getPrefixesArraySize(); ++i)
            size += strlen(option->getPrefixes(i).getNetwork()) + 1;

        length = size + TL_SIZE;
    }
    else if(dynamic_cast<CDPOptionODRDef *> (opt))
    {
        CDPOptionODRDef *option = dynamic_cast<CDPOptionODRDef *> (opt);
        length = strlen(option->getDefaultRoute()) + TL_SIZE;
    }

    return length;
}


void CDPUpdate::setOptionLength(TLVOptionBase *opt)
{
    opt->setLength(getOptionLength(opt));
}

/**
 * Count the standard IP checksum for message.
 */
uint16_t CDPUpdate::countChecksum()
{
    std::string a;
    const char *serialized;

    a += version;
    a += (char)ttl;
    for(unsigned int i=0; i < this->getOptionArraySize(); i++)
    {
        TLVOptionBase *opt = &this->getOption(i);
        if(dynamic_cast<CDPOptionDevId *> (opt))
        {
            CDPOptionDevId *option = dynamic_cast<CDPOptionDevId *> (opt);
            a += option->getType();
            a += option->getLength();
            a += option->getValue();
        }
        else if(dynamic_cast<CDPOptionPortId *> (opt))
        {
            CDPOptionPortId *option = dynamic_cast<CDPOptionPortId *> (opt);
            a += option->getType();
            a += option->getLength();
            a += option->getValue();
        }
        else if(dynamic_cast<CDPOptionVersion *> (opt))
        {
            CDPOptionVersion *option = dynamic_cast<CDPOptionVersion *> (opt);
            a += option->getType();
            a += option->getLength();
            a += option->getValue();
        }
        else if(dynamic_cast<CDPOptionPlatform *> (opt))
        {
            CDPOptionPlatform *option = dynamic_cast<CDPOptionPlatform *> (opt);
            a += option->getType();
            a += option->getLength();
            a += option->getValue();
        }
        else if(dynamic_cast<CDPOptionVtp *> (opt))
        {
            CDPOptionVtp *option = dynamic_cast<CDPOptionVtp *> (opt);
            a += option->getType();
            a += option->getLength();
            a += option->getValue();
        }
        else if(dynamic_cast<CDPOptionCapa *> (opt))
        {
            CDPOptionCapa *option = dynamic_cast<CDPOptionCapa *> (opt);
            a += option->getType();
            a += option->getLength();
            a += option->getCap(0);
            a += option->getCap(1);
            a += option->getCap(2);
            a += option->getCap(3);
        }
        else if(dynamic_cast<CDPOptionDupl *> (opt))
        {
            CDPOptionDupl *option = dynamic_cast<CDPOptionDupl *> (opt);
            a += option->getType();
            a += option->getLength();
            if(option->getFullDuplex())
                a += (char) 1;
            else
                a += (char) 0;
        }
        else if(dynamic_cast<CDPOptionAddr *> (opt))
        {
            CDPOptionAddr *option = dynamic_cast<CDPOptionAddr *> (opt);
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
        else if(dynamic_cast<CDPOptionPref *> (opt))
        {
            CDPOptionPref *option = dynamic_cast<CDPOptionPref *> (opt);
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
        else if(dynamic_cast<CDPOptionODRDef *> (opt))
        {
            CDPOptionODRDef *option = dynamic_cast<CDPOptionODRDef *> (opt);
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
