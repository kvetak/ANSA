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
/**
* @file RIPngInterface.cc
* @author Jiri Trhlik (mailto:jiritm@gmail.com), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
* @brief RIPng Interface
* @detail Represents RIPng interface
*/

#include "RIPngInterface.h"
#include "RIPngProcess.h"

namespace RIPng
{

Interface::Interface(int intId, RIPngProcess *process) :
    id(intId),
    pProcess(process),
    pOutputSocket(NULL)
{
    bDefaultInformation = false;
    bDefaultRouteOnly = false;
    bDefaultRouteMetric = 1;

    iMetricOffset = 1;

    disablePassive();
    enableSplitHorizon();
    disablePoisonReverse();
}

Interface::~Interface()
{
}


void Interface::setDefaultInformationOriginate()
{
    bDefaultInformation = true;
    bDefaultRouteOnly = false;
    bDefaultRouteMetric = 1;
    pProcess->setDefaultInformation(true);
}

void Interface::setDefaultInformationOnly()
{
    bDefaultInformation = true;
    bDefaultRouteOnly = true;
    bDefaultRouteMetric = 1;
    pProcess->setDefaultInformation(true);
}

void Interface::noDefaultInforamtion()
{
    bDefaultInformation = false;
    bDefaultRouteOnly = false;
    pProcess->setDefaultInformation(false);
}

} /* namespace RIPng */
