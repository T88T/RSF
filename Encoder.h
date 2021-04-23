/*
 * Encoder.h
 *
 *  Created on: 10 mars 2021
 *      Author: TooT
 */

#ifndef ENCODER_H_
#define ENCODER_H_

#include "Arduino.h"
#include "UI.h"

#define DEBOUNCE_TIMER 300;

class Encoder
{
	public:
		Encoder(uint8_t pinA, uint8_t pinB, UI* ui);
		Encoder(uint8_t pinA, uint8_t pinB, uint8_t pinS, UI* ui);

		int8_t getPosition();
		int8_t getDirection();

		enum Direction{ClockWise = 1, CClockWise = -1};

		void CWiseAction();
		void CCWiseAction();
		void SelectAction();

		void update();

	private:
		uint8_t mPinA;
		uint8_t mPinB;
		uint8_t mPinS;

		int8_t mDirection;
		int8_t mPosition;

		int8_t mState;

		uint32_t mPrevDebounce;
		uint32_t mDebounceTimer;

		bool pinAState;
		bool pinBState;
		bool pinSState;

		bool pinAPrevState;
		bool pinBPrevState;
		bool pinSPrevState;

		UI* mUI;
};

#endif /* ENCODER_H_ */
