//
// Copyright (C) 2010 Martin Danko
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


#include <omnetpp.h>
#include "App_custum.h"


Register_Class(custom);


bool custom::loadConfig(const cXMLElement& appConfig)
{
  
  cXMLElement* element;

  element = appConfig.getFirstChildWithTag("packets_per_second");
  if(element != NULL)
    pps = atof(element->getNodeValue());
    
  element = appConfig.getFirstChildWithTag("packet_size");
  if(element != NULL)
    size = atoi(element->getNodeValue());
    
  element = appConfig.getFirstChildWithTag("time_distribution");
  if(element != NULL)
    tDistr = element->getNodeValue();
    
  element = appConfig.getFirstChildWithTag("size_distribution");
  if(element != NULL)
    sDistr = element->getNodeValue();
  
  if(pps == 0.0 || size == 0)
    return false;
  
  return true;
}

int custom::getDefaultPort()
{
  return 1024;
}

int custom::anotherEncapsulationOverhead()
{
  return 0;
}

double custom::getNextPacketTime()
{
  if(tDistr == "normal")
  {
    return normal(1.0/pps, 1.0/(pps*10));
  }
  else if (tDistr == "exponential")
  {
    return exponential(1.0/pps);
  }
  else
  {
    return 1.0/pps; 
  }
}
    
int custom::getPacketSize()
{
  if(tDistr == "normal")
  {
    return (int) normal(size, size/10);
  }
  else if (tDistr == "exponential")
  {
    return (int) exponential(size);
  }
  else
  {
    return size; 
  }
}

