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


#ifndef __INET_APP_CUSTOM_H
#define __INET_APP_CUSTOM_H

#include "ITrafGenApplication.h"


class INET_API custom : public ITrafGenApplication
{
  private:
    double pps;
    int size;
    std::string tDistr;
    std::string sDistr;

  public:
    custom() {pps = 0.0; size = 0; tDistr = "constant"; sDistr = "constant";}
    
    virtual bool loadConfig(const cXMLElement& appConfig);

    virtual int getDefaultPort();
    
    virtual double getNextPacketTime();
    
    virtual int getPacketSize();
    
    virtual int anotherEncapsulationOverhead();
};

#endif

