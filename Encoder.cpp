/*
 * Encoder.cpp
 *
 *  Created on: 10 mars 2021
 *      Author: TooT
 */

#include "Encoder.h"

Encoder::Encoder(uint8_t pinA, uint8_t pinB, UI* ui) : mPinA(pinA), mPinB(pinB), mUI(ui)
{
	pinMode(mPinA, INPUT_PULLUP);
	pinMode(mPinB, INPUT_PULLUP);

	pinAState = true;
	pinBState = true;
	pinSState = true;

	pinAPrevState = true;
	pinBPrevState = true;
	pinSPrevState = true;

	mDirection = 0;
	mPosition = 0;
	mState = 0;

	mPinS = 0;

	mPrevDebounce = 0;
	mDebounceTimer = 0;
}


Encoder::Encoder(uint8_t pinA, uint8_t pinB, uint8_t pinS, UI* ui) : mPinA(pinA), mPinB(pinB), mPinS(pinS), mUI(ui)
{
	pinMode(mPinA, INPUT_PULLUP);
	pinMode(mPinB, INPUT_PULLUP);
	pinMode(mPinS, INPUT_PULLUP);

	pinAState = true;
	pinBState = true;
	pinSState = true;

	pinAPrevState = true;
	pinBPrevState = true;
	pinSPrevState = true;

	mDirection = 0;
	mPosition = 0;
	mState = 0;

	mPrevDebounce = 0;
	mDebounceTimer = 0;
}

int8_t Encoder::getPosition()
{
	int8_t t = mPosition;
	mPosition = 0;

	return t;
}


int8_t Encoder::getDirection()
{
	int8_t t = mDirection;
	mDirection= 0;

	return t;
}

void Encoder::update()
{
	pinAState = digitalRead(mPinA);
	pinBState = digitalRead(mPinB);

	if(mPinS != 0) // Encoder push button is used
	{
		pinSState = digitalRead(mPinS);

		if(pinSState != pinSPrevState && pinSState == false)
		{
			if(mDebounceTimer != 0)
			{
				uint32_t dt = millis() - mPrevDebounce;
				dt > mDebounceTimer ? mDebounceTimer = 0 : mDebounceTimer -= dt;
				Serial.println("Debounced");
				mPrevDebounce = millis();
			}

			if(mDebounceTimer == 0)
			{
				mDebounceTimer = DEBOUNCE_TIMER;
				mPrevDebounce = millis();
				mUI->requestAction(UI::Select);
				Serial.println("Pushed !");
			}
		}

	}

	if(pinAState == pinAPrevState && pinBState == pinBPrevState)
		return;

	if(mState == 0)
	{
		//_	     ___
		// \__
		//  A     B
		if(pinAState == false && pinAPrevState == true && pinBState == true && pinBPrevState == true)
		{
			mState = 1;
		}

		//_	     ___
		// \__
		//  B     A
		else if(pinAState == true && pinAPrevState == true && pinBState == false && pinBPrevState == true)
		{
			mState = -1;
		}
	}

	else if(mState == 1)
	{
		//	    __
		// ___    \__
		//  A     B
		if(pinAState == false && pinAPrevState == false && pinBState == false && pinBPrevState == true)
		{
			mState = 2;
			pinBPrevState = pinBState;
		}

		else
			mState = 0;
	}

	else if(mState == 2)
	{
		//	 _
		//__/	 ___
		//  A     B
		if(pinAState == true && pinAPrevState == false && pinBState == false && pinBPrevState == false)
		{
			mState = 3;
		}
		else
			mState = 0;
	}

	else if(mState == 3)
	{
		// ___	   __
		// 		 _/
		//  A     B
		if(pinAState == true && pinAPrevState == true && pinBState == true && pinBPrevState == false)
		{
			mState = 0;
			mDirection = ClockWise;

			mUI->requestAction(UI::Next);

			mPosition ++;
		}
		else
			mState = 0;
	}

	else if(mState == -1)
	{
		//	    __
		// ___    \__
		//  B     A
		if(pinAState == false && pinAPrevState == true && pinBState == false && pinBPrevState == false)
		{
			mState = -2;
		}

		else
			mState = 0;
	}

	else if(mState == -2)
	{
		//	 _
		//__/	 ___
		//  B     A
		if(pinAState == false && pinAPrevState == false && pinBState == true && pinBPrevState == false)
		{
			mState = -3;
		}
		else
			mState = 0;
	}

	else if(mState == -3)
	{
		// ___	   __
		// 		 _/
		//  B     A
		if(pinAState == true && pinAPrevState == false && pinBState == true && pinBPrevState == true)
		{
			mState = 0;
			mDirection = CClockWise;

			mUI->requestAction(UI::Prev);

			mPosition--;
		}
		else
			mState = 0;
	}

	pinAPrevState = pinAState;
	pinBPrevState = pinBState;
	pinSPrevState = pinSState;

}
