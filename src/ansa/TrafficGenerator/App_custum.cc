//
// Copyright (C) 2011 Martin Danko
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

/*
 * loadConfig(): 
 * Funkcia nacita konfiguraciu aplikacie zo XML suboru 
 * @param appConfig	- XML blok s konfiguraciu generovanej aplikacie 
 * @return - Vrati true bola konfiguracia spravne nacitana
 */
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
    return false; // chyba nacitania
  
  return true;
}

/*
 * getDefaultPort(): 
 * Funkcia vracia hodnotu implicitneho portu aplikacie  
 * @return - Vrati implicitny port
 */ 
int custom::getDefaultPort()
{
  return 1024;
}

/*
 * getNextPacketTime(): 
 * Funkcia vrati velkost hlaviciek nad ramec transportneho protokolu
 * V tejto aplikacii nie je ziadna encapsulacia navyse 
 * @return - Vrati nulu
 */
int custom::anotherEncapsulationOverhead()
{
  return 0;
}

/*
 * getNextPacketTime(): 
 * Funkcia na zaklade definovaneho rozlozenia uci cas za aky
 * sa bude generovat dalsi paket  
 * @return - Vrati cas genetovania dalsieho paketu
 */  
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

/*
 * getPacketSize(): 
 * Funkcia podla definovaneho rozlozenia vrati velkost vygenerovaneho paketu
 * @return - Vrati velkost vygenerovaneho paketu
 */      
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

