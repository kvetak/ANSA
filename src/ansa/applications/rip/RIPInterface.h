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
* @file RIPInterface.h
* @author Jiri Trhlik (mailto:), Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
* @brief
* @detail
*/

#ifndef RIPINTERFACE_H_
#define RIPINTERFACE_H_

namespace RIP
{

class Interface
{
  public:
    Interface(int intId);
    virtual ~Interface();

  protected:
    int id;                     ///< id of the interface as is in interfaceTable
    bool bPassive;              ///< if interface is passive
    bool bSplitHorizon;         ///< if split horizon is enabled on this interface
    bool bPoisonReverse;        ///< if poison reverse is enabled on this interface

  public:
    void enablePassive()   { bPassive = true; }
    void disablePassive() { bPassive = false; }
    bool isPassive() { return bPassive; }

    void enableSplitHorizon()  { bSplitHorizon = true; }
    void disableSplitHorizon()  { bSplitHorizon = false; }
    bool isSplitHorizon()   { return bSplitHorizon; }

    void enablePoisonReverse()  { bPoisonReverse = true; }
    void disablePoisonReverse()  { bPoisonReverse = false; }
    bool isPoisonReverse()   { return bPoisonReverse; }

    int getId() { return id; }
};

} /* namespace RIP */

#endif /* RIPINTERFACE_H_ */
