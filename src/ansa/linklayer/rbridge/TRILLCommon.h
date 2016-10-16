// Copyright (C) 2016 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
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
 * @file TRILLCommon.h
 * @author Marcel Marek (mailto:imarek@fit.vutbr.cz)
 * @date 16.10.2016
 * @brief TRILL common classes.
 * @detail TRILL common classes.
  */


#ifndef ANSA_LINKLAYER_RBRIDGE_TRILLCOMMON_H_
#define ANSA_LINKLAYER_RBRIDGE_TRILLCOMMON_H_

#include <iostream>
#include <string>

#include "ansa/networklayer/isis/ISISCommon.h"

namespace inet {

static const int TRILL_NICKNAME_STRING_SIZE = 4; //4 chars

class TRILLNickname {
    protected:
    uint16 nickname;

    public:
    TRILLNickname() {

      nickname = 0;
    }

    TRILLNickname(SystemID sysID){
      nickname = sysID.toInt() && 0xFFFF;
    }
    uint16 getNickname() const {
        return nickname;
    }

    virtual std::string str() const
    {

        char buf[TRILL_NICKNAME_STRING_SIZE + 1];
        sprintf(buf, "%04X", nickname);
        buf[TRILL_NICKNAME_STRING_SIZE] = 0;
        return std::string(buf);
    }


    bool operator==(const TRILLNickname& nick) const {return nickname == nick.getNickname();}
    bool operator!=(const TRILLNickname& nick) const {return nickname != nick.getNickname();}

    bool operator<(const TRILLNickname& nick) const {return nickname < nick.getNickname();}
    bool operator<=(const TRILLNickname& nick) const {return nickname <= nick.getNickname();}

    bool operator>(const TRILLNickname& nick) const {return nickname > nick.getNickname();}
    bool operator>=(const TRILLNickname& nick) const {return nickname >= nick.getNickname();}

};

inline std::ostream& operator<<(std::ostream& os, const TRILLNickname& nickname)
{
    return os << nickname.str();
}


} //end namespace inet



#endif /* ANSA_LINKLAYER_RBRIDGE_TRILLCOMMON_H_ */
