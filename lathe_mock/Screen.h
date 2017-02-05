#ifndef SCREEEN_H
#define SCREEEN_H
#include <SPI_anything.h>
#include <Def.h>

void sendScreenUpdate(bool force = false) {
	int val;
	for (int i=0; i< stateCount; i++) {
		if (state[i] != oldState[i] || force) {
			val = state[i];
			if (i == modeEncoderValue) {
				if (state[actionMode] == MODE_ANGLE && val == 6) {
					val = angle;
				} else if (state[actionMode] == MODE_THREAD && val > 5) {
					val += 2;
				}				
			}
			SPI_sendMessage(i, val);
			oldState[i] = state[i];
		}
	}
}

#endif







