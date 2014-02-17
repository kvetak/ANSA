/*
 * STPMACCompare.h
 *
 *  Created on: 16.4.2011
 *      Author: aranel
 */

#ifndef __STPMACCOMPARE_H__
#define __STPMACCOMPARE_H__

#include "MACAddress.h"

bool operator<(const MACAddress& a, const MACAddress& b) {
	if (a.compareTo(b) == -1) {
		return true;
	}
	return false;
}

bool operator>(const MACAddress& a, const MACAddress& b) {
	if (a.compareTo(b) == 1) {
		return true;
	}
	return false;
}

bool operator<=(const MACAddress& a, const MACAddress& b) {
	if (a.compareTo(b) != 1) {
		return true;
	}
	return false;
}

bool operator>=(const MACAddress& a, const MACAddress& b) {
	if (a.compareTo(b) != -1) {
		return true;
	}
	return false;
}

#endif /* STPMACCOMPARE_H_ */
