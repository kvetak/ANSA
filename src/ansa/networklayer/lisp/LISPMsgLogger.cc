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


#include <LISPMsgLogger.h>

Define_Module(LISPMsgLogger);

LISPMsgLogger::LISPMsgLogger() {

}

LISPMsgLogger::~LISPMsgLogger() {
    MsgLogger.clear();
}

void LISPMsgLogger::addMsg(LISPMsgEntry::EMsgType type, unsigned long nonce, IPvXAddress addr, bool flag) {
    MsgLogger.push_back( LISPMsgEntry(type, nonce, addr, simTime(), flag) );

    if (flag) {
        msgsent++;
        emit(sigSend, true);
    }
    else {
        msgrecv++;
        emit(sigRecv, true);
    }
    emit(sigMsg, (int)type);

    updateDisplayString();
}

LISPMsgEntry* LISPMsgLogger::findMsg(LISPMsgEntry::EMsgType type, unsigned long nonce) {
    for (MsgItem it = MsgLogger.begin(); it != MsgLogger.end(); ++it) {
        if (it->getNonce() == nonce && it->getType() == type)
            return &(*it);
    }
    return NULL;
}

MessageLog& LISPMsgLogger::getMsgLogger() {
    return MsgLogger;
}

void LISPMsgLogger::initialize(int stage) {
    if (stage < numInitStages() - 1)
        return;
    msgsent = 0;
    msgrecv = 0;
    updateDisplayString();
    initSignals();
    WATCH_LIST(MsgLogger);
}

void LISPMsgLogger::initSignals() {
    sigSend     = registerSignal(SIG_LOG_SEND);
    sigRecv     = registerSignal(SIG_LOG_RECV);
    sigMsg      = registerSignal(SIG_LOG_MSG);
}

void LISPMsgLogger::handleMessage(cMessage* msg) {
    error("LISP MsgLoogger should not receive any messages");
}

void LISPMsgLogger::updateDisplayString() {
    if (!ev.isGUI())
        return;
    std::ostringstream description;
    description << msgsent << " sent" << endl
                << msgrecv << " recv" ;
    this->getDisplayString().setTagArg("t", 0, description.str().c_str());
    this->getDisplayString().setTagArg("t", 1, "t");
}
