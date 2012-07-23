// FIT VUT 2012
//
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

#include "ISISProcess.h"
//Register_Class(ISISProcess);

ISISProcess::ISISProcess() {
    // TODO Auto-generated constructor stub
    this->ISISIft = new std::vector<ISISinterface>();

}


std::string ISISProcess::getProcessTag() const
{
    return processTag;
}

short ISISProcess::getIsType() const
{
    return isType;
}

unsigned long ISISProcess::getLspMaxLifetime() const
{
    return lspMaxLifetime;
}

unsigned long ISISProcess::getLspRefreshInter() const
{
    return lspRefreshInter;
}

void ISISProcess::setIsType(short  isType)
{
    this->isType = isType;
}

void ISISProcess::setLspMaxLifetime(unsigned long  lspMaxLifetime)
{
    this->lspMaxLifetime = lspMaxLifetime;
}

const char *ISISProcess::getNetAddr()
{
    return netAddr;
}

std::vector<ISISadj> ISISProcess::getAdjL1Table() const
{
    return adjL1Table;
}

std::vector<ISISadj> ISISProcess::getAdjL2Table() const
{
    return adjL2Table;
}

const unsigned char *ISISProcess::getAreaId() const
{
    return areaId;
}

std::vector<LSPrecord> ISISProcess::getL1Lsp() const
{
    return L1LSP;
}

std::vector<LSPrecord> ISISProcess::getL2Lsp() const
{
    return L2LSP;
}

const unsigned char *ISISProcess::getNsel() const
{
    return NSEL;
}

void ISISProcess::setAreaId(unsigned char *areaId)
{
    this->areaId = areaId;
}

void ISISProcess::setNsel(unsigned char *nsel)
{
    NSEL = nsel;
}

void ISISProcess::setSysId(unsigned char *sysId)
{
    this->sysId = sysId;
}

void ISISProcess::setIsisIft(std::vector<ISISinterface> *isisIft)
{
    ISISIft = isisIft;
}

std::vector<ISISinterface> *ISISProcess::getIsisIft() const
{
    return ISISIft;
}

const unsigned char *ISISProcess::getSysId() const
{
    return sysId;
}

void ISISProcess::setNetAddr(const char *netAddr)
{
    this->netAddr = netAddr;
}

void ISISProcess::setLspRefreshInter(unsigned long  lspRefreshInter)
{
    this->lspRefreshInter = lspRefreshInter;
}

void ISISProcess::setProcessTag(std::string processLabel)
{
    this->processTag = processLabel;
}

ISISProcess::~ISISProcess() {
    // TODO Auto-generated destructor stub
}

