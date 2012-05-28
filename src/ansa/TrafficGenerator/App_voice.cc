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
#include "App_voice.h"


Register_Class(voice);

/*
 * loadConfig(): 
 * Funkcia nacita konfiguraciu aplikacie zo XML suboru 
 * @param appConfig	- XML blok s konfiguraciu generovanej aplikacie 
 * @return - Vrati true bola konfiguracia spravne nacitana
 */
bool voice::loadConfig(const cXMLElement& appConfig)
{
  
  cXMLElement* element;

  element = appConfig.getFirstChildWithTag("codec");
  if(element != NULL)
  {// pouzity kodek
    std::string codec = element->getNodeValue();
    if(codec == "g711" || codec == "G.711")
      codecRate = 64000;
    else if(codec == "g729" || codec == "G.729")
      codecRate = 8000;
  }
  
  element = appConfig.getFirstChildWithTag("codec_rate");
  if(element != NULL) // explicitne definavana bitova rychlost
    codecRate =  atoi(element->getNodeValue());
    
  element = appConfig.getFirstChildWithTag("packets_per_second");
  if(element != NULL)
  {
    std::string ppsString = element->getNodeValue();
    if(ppsString == "default")
      pps = 50.0;
    else
      pps = atof(element->getNodeValue());
  }
    
  element = appConfig.getFirstChildWithTag("vad");
  if(element != NULL)
  { // technologia VAD bude pouzita
    std::string vadString = element->getNodeValue();
    if(vadString == "true" || vadString == "True" || vadString == "TRUE")
      vad = true;
  }
  
  if(pps == 0.0 || codecRate == 0)
    return false; // chyba nacitania
  
  return true;
}

/*
 * getDefaultPort(): 
 * Funkcia vracia hodnotu implicitneho portu aplikacie  
 * @return - Vrati implicitny port VoIP
 */ 
int voice::getDefaultPort()
{
  return 16384;
}

/*
 * getNextPacketTime(): 
 * Funkcia vrati velkost hlaviciek nad ramec transportneho protokolu
 * U VoIP je to velkost RTP hlavicky   
 * @return - Vrati velkost RTP hlavicky
 */ 
int voice::anotherEncapsulationOverhead()
{
  return 12;  // rtp hlavicka
}

/*
 * getNextPacketTime(): 
 * Funkcia na zaklade poctu paketov za sekundu uci cas za aky
 * sa bude generovat dalsi paket  
 * @return - Vrati cas genetovania dalsieho paketu
 */  
double voice::getNextPacketTime()
{
  if(vad)
  { // VAD sposobi, ze je generovanych len 70% paketov 
    double time = 1.0/pps;
    while(intuniform(0, 10) > 7)
      time += 1.0/pps;
    
    return time;
  }
  return 1.0/pps; 
}

/*
 * getPacketSize(): 
 * Funkcia na zaklade kodeku a poctu paketov za sekundu urci velkost 
 * generovaneho paketu 
 * @return - Vrati velkost vygenerovaneho paketu
 */    
int voice::getPacketSize()
{
  return  codecRate/(8 * pps);
}

