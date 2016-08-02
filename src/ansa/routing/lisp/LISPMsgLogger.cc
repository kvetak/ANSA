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
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */


#include "ansa/routing/lisp/LISPMsgLogger.h"

namespace inet {
Define_Module(LISPMsgLogger);

LISPMsgLogger::LISPMsgLogger() :
        msgsent(0), msgrecv(0), sizesent(0), sizerecv(0)
{
}

LISPMsgLogger::~LISPMsgLogger() {
    msgsent = 0;
    msgrecv = 0;
    sizesent = 0;
    sizerecv = 0;

    MsgLogger.clear();
}

void LISPMsgLogger::addMsg(LISPMessage* lispmsg, LISPMsgEntry::EMsgType msgtype, L3Address addr, bool flag) {
    MsgLogger.push_back(
            LISPMsgEntry(msgtype,
                         lispmsg->getNonce(),
                         addr,
                         simTime(),
                         flag,
                         lispmsg->getByteLength()) );

    recordStatistics(lispmsg, (int)msgtype, flag);

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

    updateDisplayString();
    initSignals();

    WATCH(msgsent);
    WATCH(msgrecv);
    WATCH(sizesent);
    WATCH(sizerecv);
    WATCH_LIST(MsgLogger);
}

void LISPMsgLogger::initSignals() {
    sigSend     = registerSignal(SIG_LOG_SEND);
    sigRecv     = registerSignal(SIG_LOG_RECV);
    sigSizeSend = registerSignal(SIG_LOG_SIZESEND);
    sigSizeRecv = registerSignal(SIG_LOG_SIZERECV);
    sigMsg      = registerSignal(SIG_LOG_MSG);
}

void LISPMsgLogger::handleMessage(cMessage* msg) {
    error("LISP MsgLoogger should not receive any messages");
}

void LISPMsgLogger::updateDisplayString() {
    if (!getEnvir()->isGUI())
        return;
    std::ostringstream description;
    description << msgsent << " sent" << endl
                << msgrecv << " recv" ;
    this->getDisplayString().setTagArg("t", 0, description.str().c_str());
    this->getDisplayString().setTagArg("t", 1, "t");
}

void LISPMsgLogger::recordStatistics(LISPMessage* lispmsg, int msgtype, bool flag) {
    if (flag) {
        msgsent++;
        sizesent += lispmsg->getByteLength();
        emit(sigSend, true);
        emit(sigSizeSend, (int)lispmsg->getByteLength());
    }
    else {
        msgrecv++;
        sizerecv += lispmsg->getByteLength();
        emit(sigRecv, true);
        emit(sigSizeRecv, (int)lispmsg->getByteLength());
    }

    emit(sigMsg, msgtype);
}

}
