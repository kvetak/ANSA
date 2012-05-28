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

#ifndef __ANSA_DSCPTTLCORRECTOR_H
#define __ANSA_DSCPTTLCORRECTOR_H

#include <omnetpp.h>
#include "TrafGenAccess.h"

/* Trieda definujuca modul prekorekciu TTL a DSCP */ 
class DscpTtlCorrector : public cSimpleModule
{
  private:
  
    TrafGen*     tg; // ukazovatel na modul traffic generatoru
  
  protected:
  
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);

    
  public:
  

};

#endif
