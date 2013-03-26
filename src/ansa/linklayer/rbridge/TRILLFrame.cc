// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
 * @file TRILLFrame.cc
 * @author Marcel Marek (mailto:xscrew02@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @date 16.3.2012
 * @brief
 * @detail
 * @todo
 */
#include "TRILLFrame.h"
#include <iostream>
#include <sstream>
//#include "TRILLFrame_m.h"
Register_Class(TRILLFrame);

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// Another default rule (prevents compiler from choosing base class' doPacking())
template<typename T>
void doPacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doPacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}

template<typename T>
void doUnpacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doUnpacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}






TRILLFrame::TRILLFrame(const char *name, int kind) : TRILLFrame_Base(name,kind)
{
    this->Ethertype_var = 0;
    this->version_var = 0;
    this->reserved_var = 0;
    this->multiDest_var = 0;
    this->opLength_var = 0;
    this->hopCount_var = 0;
    this->egressRBNickname_var = 0;
    this->ingressRBNickname_var = 0;
    options_arraysize = 0;
    this->options_var = 0;
}

TRILLFrame::TRILLFrame(const TRILLFrame& other) : TRILLFrame_Base(other)
{
    options_arraysize = 0;
    this->options_var = 0;
    copy(other);
}

TRILLFrame::~TRILLFrame()
{
    delete [] options_var;
}

TRILLFrame& TRILLFrame::operator=(const TRILLFrame& other)
{
    if (this==&other) return *this;
    cPacket::operator=(other);
    copy(other);
    return *this;
}

void TRILLFrame::copy(const TRILLFrame& other)
{
    this->Ethertype_var = other.Ethertype_var;
    this->version_var = other.version_var;
    this->reserved_var = other.reserved_var;
    this->multiDest_var = other.multiDest_var;
    this->opLength_var = other.opLength_var;
    this->hopCount_var = other.hopCount_var;
    this->egressRBNickname_var = other.egressRBNickname_var;
    this->ingressRBNickname_var = other.ingressRBNickname_var;
    delete [] this->options_var;
    this->options_var = (other.options_arraysize==0) ? NULL : new uint32_t[other.options_arraysize];
    options_arraysize = other.options_arraysize;
    for (unsigned int i=0; i<options_arraysize; i++)
        this->options_var[i] = other.options_var[i];
}

void TRILLFrame::parsimPack(cCommBuffer *b)
{
    cPacket::parsimPack(b);
    doPacking(b,this->Ethertype_var);
    doPacking(b,this->version_var);
    doPacking(b,this->reserved_var);
    doPacking(b,this->multiDest_var);
    doPacking(b,this->opLength_var);
    doPacking(b,this->hopCount_var);
    doPacking(b,this->egressRBNickname_var);
    doPacking(b,this->ingressRBNickname_var);
    b->pack(options_arraysize);
    doPacking(b,this->options_var,options_arraysize);
}

void TRILLFrame::parsimUnpack(cCommBuffer *b)
{
    cPacket::parsimUnpack(b);
    doUnpacking(b,this->Ethertype_var);
    doUnpacking(b,this->version_var);
    doUnpacking(b,this->reserved_var);
    doUnpacking(b,this->multiDest_var);
    doUnpacking(b,this->opLength_var);
    doUnpacking(b,this->hopCount_var);
    doUnpacking(b,this->egressRBNickname_var);
    doUnpacking(b,this->ingressRBNickname_var);
    delete [] this->options_var;
    b->unpack(options_arraysize);
    if (options_arraysize==0) {
        this->options_var = 0;
    } else {
        this->options_var = new uint32_t[options_arraysize];
        doUnpacking(b,this->options_var,options_arraysize);
    }
}

uint16_t TRILLFrame::getEthertype() const
{
    return Ethertype_var;
}

void TRILLFrame::setEthertype(uint16_t Ethertype)
{
    this->Ethertype_var = Ethertype;
}

unsigned short TRILLFrame::getVersion() const
{
    return version_var;
}

void TRILLFrame::setVersion(unsigned short version)
{
    this->version_var = version;
}

unsigned short TRILLFrame::getReserved() const
{
    return reserved_var;
}

void TRILLFrame::setReserved(unsigned short reserved)
{
    this->reserved_var = reserved;
}

bool TRILLFrame::getMultiDest() const
{
    return multiDest_var;
}

void TRILLFrame::setMultiDest(bool multiDest)
{
    this->multiDest_var = multiDest;
}

uint8_t TRILLFrame::getOpLength() const
{
    return opLength_var;
}

void TRILLFrame::setOpLength(uint8_t opLength)
{
    this->opLength_var = opLength;
}

uint8_t TRILLFrame::getHopCount() const
{
    return hopCount_var;
}

void TRILLFrame::setHopCount(uint8_t hopCount)
{
    this->hopCount_var = hopCount;
}

uint16_t TRILLFrame::getEgressRBNickname() const
{
    return egressRBNickname_var;
}

void TRILLFrame::setEgressRBNickname(uint16_t egressRBNickname)
{
    this->egressRBNickname_var = egressRBNickname;
}

uint16_t TRILLFrame::getIngressRBNickname() const
{
    return ingressRBNickname_var;
}

void TRILLFrame::setIngressRBNickname(uint16_t ingressRBNickname)
{
    this->ingressRBNickname_var = ingressRBNickname;
}

void TRILLFrame::setOptionsArraySize(unsigned int size)
{
    uint32_t *options_var2 = (size==0) ? NULL : new uint32_t[size];
    unsigned int sz = options_arraysize < size ? options_arraysize : size;
    for (unsigned int i=0; i<sz; i++)
        options_var2[i] = this->options_var[i];
    for (unsigned int i=sz; i<size; i++)
        options_var2[i] = 0;
    options_arraysize = size;
    delete [] this->options_var;
    this->options_var = options_var2;
}

unsigned int TRILLFrame::getOptionsArraySize() const
{
    return options_arraysize;
}

uint32_t TRILLFrame::getOptions(unsigned int k) const
{
    if (k>=options_arraysize) throw cRuntimeError("Array of size %d indexed by %d", options_arraysize, k);
    return options_var[k];
}

void TRILLFrame::setOptions(unsigned int k, uint32_t options)
{
    if (k>=options_arraysize) throw cRuntimeError("Array of size %d indexed by %d", options_arraysize, k);
    this->options_var[k] = options;
}



