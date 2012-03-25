/*
 * STPTimer.h
 *
 *  Created on: 19.4.2011
 *      Author: aranel
 */

#ifndef STPTIMER_H_
#define STPTIMER_H_

#define STPTIMERCOUNT 8

class STPTimer
{
  public:
	STPTimer();
	virtual ~STPTimer();

	typedef enum {
		NONE = -1,
		EDGEDELAYWHILE = 0,
		FDWHILE = 1,
		HELLOWHEN = 2,
		// MDELAYWHILE = 3,
		RBWHILE = 4,
		RCDVINFOWHILE = 5,
		RRWHILE = 6,
		TCWHILE = 7,
	} eSTPTimer;


	void setInitValue(eSTPTimer timer, int value);
	int getInitValue(eSTPTimer timer);
	int getValue(eSTPTimer timer);


	bool existTimedOut();
	bool isTimedOut(eSTPTimer timer);

  private:

/* --- TIMERS --- */
	int edgeDelayWhile; // 17.17.1 considering edge port timer
	int fdWhile; // 17.17.2 forwarding port state transition delay
	int helloWhen; // 17.17.3 hello timer, at least one BPDU at helloWhen
	// int mdelayWhile; // 17.17.4 protocol migration timer (NOT CONSIDERED)
	int rbWhile; // 17.17.5 recent backup, twice as hello timer, while port is backup
	int rcdvInfoWhile; // 17.17.6 received info, used for aging received info by this port
	int rrWhile; // 17.17.7 recent root
	int tcWhile; // 17.17.8 topology change, TCN are sent while this timer running
/* --- INITIAL VALUES --- */
	int initEdgeDelayWhile;
	int initFdWhile;
	int initHelloWhen;
	// int initMdelayWhile;
	int initRbWhile; /* (!) TWICE AS HELLO TIMER, but for unify */
	int initRcdvInfoWhile;
	int initRrWhile;
	int initTcWhile;


/* --- INDEX TIMER ADDRESSING --- */
	int * timerValue[STPTIMERCOUNT];
	int * initValue[STPTIMERCOUNT];

	/* set default values for initValues */
	void setDefaultInitValues();


  protected:




};

#endif /* STPTIMER_H_ */
