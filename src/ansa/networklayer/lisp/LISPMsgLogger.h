//
// Copyright (C) 2013, 2014 Brno University of Technology
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
 * @author Vladimir Vesely / ivesely@fit.vutbr.cz / http://www.fit.vutbr.cz/~ivesely/
 */


#ifndef LISPMSGLOGGER_H_
#define LISPMSGLOGGER_H_

#include "LISPMsgEntry.h"

typedef std::list<LISPMsgEntry> MessageLog;
typedef MessageLog::iterator MsgItem;
typedef MessageLog::const_iterator MsgCItem;

class LISPMsgLogger {
  public:
    LISPMsgLogger();
    virtual ~LISPMsgLogger();

    void addMsg(LISPMsgEntry::EMsgType type, unsigned long nonce, IPvXAddress addr, bool flag);
    LISPMsgEntry* findMsg(LISPMsgEntry::EMsgType type, unsigned long nonce);
    MessageLog& getMsgLogger();

  private:
    MessageLog MsgLogger;
};

#endif /* LISPMSGLOGGER_H_ */
