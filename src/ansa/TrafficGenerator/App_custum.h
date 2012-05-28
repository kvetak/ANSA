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


#ifndef __INET_APP_CUSTOM_H
#define __INET_APP_CUSTOM_H

#include "ITrafGenApplication.h"

/* Trieda reprezentujuca obecnu aplikaciu nad TCP/UDP  */ 
class INET_API custom : public ITrafGenApplication
{
  private:
    double pps;           // pakety za sekundu
    int size;             // velkost paketu
    std::string tDistr;   // retazec casovej ditribucie
    std::string sDistr;   // retazec distribucie velkosti paketu

  public:
    custom() {pps = 0.0; size = 0; tDistr = "constant"; sDistr = "constant";}
    
    virtual bool loadConfig(const cXMLElement& appConfig);

    virtual int getDefaultPort();
    
    virtual double getNextPacketTime();
    
    virtual int getPacketSize();
    
    virtual int anotherEncapsulationOverhead();
};

#endif

