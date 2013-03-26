/*
 * STPTimer.cc
 *
 *  Created on: 19.4.2011
 *      Author: aranel
 */

#include "STPTimer.h"

STPTimer::STPTimer() {

	/* ASSIGNING INDEX ACCESS VALUES */
	timerValue[EDGEDELAYWHILE] = &edgeDelayWhile;
	timerValue[FDWHILE] = &fdWhile;
	timerValue[HELLOWHEN] = &helloWhen;
	// timerValue[MDELAYWHILE] = &mdelayWhile;
	timerValue[RBWHILE] = &rbWhile;
	timerValue[RCDVINFOWHILE] = &rcdvInfoWhile;
	timerValue[RRWHILE] = &rrWhile;
	timerValue[TCWHILE] = &tcWhile;

	initValue[EDGEDELAYWHILE] = &initEdgeDelayWhile;
	initValue[FDWHILE] = &initFdWhile;
	initValue[HELLOWHEN] = &initHelloWhen;
	// initValue[MDELAYWHILE] = &initMdelayWhile;
	initValue[RBWHILE] = &initRbWhile;
	initValue[RCDVINFOWHILE] = &initRcdvInfoWhile;
	initValue[RRWHILE] = &initRrWhile;
	initValue[TCWHILE] = &initTcWhile;

	setDefaultInitValues();
}

STPTimer::~STPTimer() {
	// TODO Auto-generated destructor stub
}


void STPTimer::setDefaultInitValues() {
	initEdgeDelayWhile = 15;
	initFdWhile = 15;
	initHelloWhen = 2;
	// initMdelayWhile; = ??
	/* (!) TWICE AS HELLO TIMER, but for unify */
	initRbWhile = 2 * initHelloWhen;
	initRcdvInfoWhile;
	initRrWhile;
	initTcWhile;




}
