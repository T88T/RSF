/*
 * UI.cpp
 *
 *  Created on: 9 mars 2021
 *      Author: TooT
 */

#include "UI.h"
#include "MusicFile.h"
#include <U8g2lib.h>
#include "vs1053_SdFat.h"

extern vs1053 MP3player;
extern Array<MusicFile*, 100> Playlist;

#define UPDATE_TIMER uint16_t(500)
#define VSCROLL_SPEED uint16_t(400)

UI::UI(U8G2_ST7920_128X64_F_SW_SPI *LCD) : mLCD(LCD)
{

	mState = States::Browsing;
	mPendingAction = None;

	//Graphic elements
	mNeedRefresh = true;

	//Menu browsing
	mNeedVScrolling = false;
	mCurrentVScroll = false;
	mVScrollTimer = 0;
	mScrollTempo = 0;

	mCurrentItem = 0;
	mCurrentPage = 0;
	mPages = 1;

	ItemDuration = 0;

	//Track Playing
	mTitle = String("");
	mDuration = 0;
	mCurrentTime = 0;
	mTrackTimer = 0;
	mCurrentControl = PlayPause;
	mPlayState = false;
	mTimeOffset = 0;
	mVolume = 192;

	mPages = 0;

	mTrack = nullptr;

}

void UI::requestAction(uint8_t Action)
{
	mPendingAction = Action;
}

void UI::fillList(Array<MusicFile*, 100> *list)
{
	mList.clear();
	for(uint8_t i = 0; i < list->size(); i++)
	{
		mList.push_back(list->at(i)->getTitle());
	}

	mPages = mList.size() / UI_ITEM_PER_PAGE;
}

String UI::timeToString(uint32_t time)
{
	return String(time/60/1000) + String(":") + String((float(time)/60.0/1000.0 - (time/60/1000))*60.0, 0);
}

void UI::nextItem()
{
	if(mState == States::Browsing)
	{
		if(mCurrentItem+1 < UI_ITEM_PER_PAGE)
		{
			if(uint8_t(mCurrentItem+1 + mCurrentPage*UI_ITEM_PER_PAGE) < mList.size())
			{
				mCurrentItem++;
				mNeedRefresh = true;

				uint8_t SelectedItem = mCurrentItem+mCurrentPage*UI_ITEM_PER_PAGE;

				if(Playlist.at(SelectedItem)->getTitle().length() > 21)
				{
					mNeedVScrolling = true;
					mCurrentVScroll = 0;
					mScrollTempo = 2;
				}

				else
					mNeedVScrolling = false;
			}
		}
		else
		{
			if(mCurrentPage+1 < mPages)
			{
				mCurrentPage++;
				mCurrentItem = 0;
				mNeedRefresh = true;

				uint8_t SelectedItem = mCurrentItem+mCurrentPage*UI_ITEM_PER_PAGE;

				if(Playlist.at(SelectedItem)->getTitle().length() > 21)
				{
					mNeedVScrolling = true;
					mCurrentVScroll = 0;
					mScrollTempo = 2;
				}

				else
					mNeedVScrolling = false;
			}
		}

		Serial.print("Item : "); Serial.print(mCurrentItem); Serial.print(" Page : "); Serial.println(mCurrentPage);
	}

	if(mState == States::Playing)
	{
		mCurrentControl = mCurrentControl == ControlCount ? ControlCount-1 : mCurrentControl +1;
		mNeedRefresh = true;
	}
}

void UI::prevItem()
{
	if(mState == States::Browsing)
	{
		if(mCurrentItem-1 >= 0)
		{
			mCurrentItem--;
			mNeedRefresh = true;

			uint8_t SelectedItem = mCurrentItem+mCurrentPage*UI_ITEM_PER_PAGE;

			if(Playlist.at(SelectedItem)->getTitle().length() > 21)
			{
				mNeedVScrolling = true;
				mCurrentVScroll = 0;
				mScrollTempo = 2;
			}

			else
				mNeedVScrolling = false;
		}
		else
		{
			if(mCurrentPage-1 >= 0)
			{
				mCurrentPage--;
				mNeedRefresh = true;
				mCurrentItem = UI_ITEM_PER_PAGE-1;

				uint8_t SelectedItem = mCurrentItem+mCurrentPage*UI_ITEM_PER_PAGE;

				if(Playlist.at(SelectedItem)->getTitle().length() > 21)
				{
					mNeedVScrolling = true;
					mCurrentVScroll = 0;
					mScrollTempo = 2;
				}

				else
					mNeedVScrolling = false;
			}

			else
				mCurrentItem = 0;
		}
	}

	if(mState == States::Playing)
	{
		mCurrentControl = mCurrentControl == 0 ? 0 : mCurrentControl -1;
		mNeedRefresh = true;
	}
}

void UI::selectItem()
{
	if(mState == States::Browsing)
	{
		uint8_t SelectedItem = mCurrentItem+mCurrentPage*UI_ITEM_PER_PAGE;

		Serial.println(SelectedItem);

		if(SelectedItem < Playlist.size())
		{
			if(MP3player.getState() == playback)
				MP3player.stopTrack();

			MP3player.setVolume(uint8_t(32));
			MP3player.playMP3(Playlist.at(SelectedItem)->getPath());

			mTrack = Playlist.at(SelectedItem);
			mState = Playing;
			mTimeOffset = 0;
			mPlayState = true;
		}
	}

	else if(mState == States::Playing)
	{
		Serial.print("Action : "); Serial.println(mCurrentControl);
		switch(mCurrentControl)
		{
			case Back:
				if((mCurrentTime+1000*mTimeOffset) < 11000)
					break;
				if(!MP3player.skip(-10000))
					mTimeOffset -=10;

				else
					MP3player.resumeMusic();

				mPlayState = true;
				break;

			case Stop:
				MP3player.stopTrack();
				mState = Browsing;
				break;

			case PlayPause:
				if(mPlayState)
					MP3player.pauseMusic();
				else
					MP3player.resumeMusic();

				mPlayState = !mPlayState;
				break;

			case Forw:
				if(!MP3player.skip(10000))
					mTimeOffset +=10;
				else
					MP3player.resumeMusic();

				mPlayState = true;
				break;

			case VolM:
				mVolume = uint8_t(MP3player.getVolume());
				uint16_t(mVolume+20) > 60 ? mVolume = 60 : mVolume += 20;
				MP3player.setVolume(mVolume);
				Serial.println(mVolume);
				break;

			case VolP:
				mVolume = uint8_t(MP3player.getVolume());
				int16_t(mVolume-20) < 0 ? mVolume = 0 : mVolume -= 20;
				MP3player.setVolume(mVolume);
				Serial.println(mVolume);
				break;
		}

		mNeedRefresh = true;
	}
}


void UI::update(uint16_t dt)
{
	if(mPendingAction != None)
	{
		switch(mPendingAction)
		{
			case Prev:
				prevItem();
				break;
			case Select:
				selectItem();
				break;
			case Next:
				nextItem();
				break;
		}

		mPendingAction = None;
	}

	if(mNeedVScrolling)
	{
		mVScrollTimer += dt;

		if(mVScrollTimer > VSCROLL_SPEED + mScrollTempo*VSCROLL_SPEED)
		{
			mScrollTempo = 0;
			mVScrollTimer = 0;
			mCurrentVScroll++;
			mNeedRefresh = true;
		}
	}

	if(mState == States::Playing && mTrack != nullptr && MP3player.getState() == playback)
	{
		mTrackTimer += dt;

		if(mTrackTimer > UPDATE_TIMER)
		{
			mTrackTimer = 0;
			mCurrentTime = MP3player.currentPosition();
			mNeedRefresh = true;
		}
	}

	if(mState == States::Playing && !MP3player.isPlaying() && mPlayState == true)
	{
		mPlayState = false;
		mState = States::Browsing;
		mNeedRefresh = true;
	}

}

void UI::draw()
{
	if(mNeedRefresh)
	{
		mLCD->clearBuffer();

		if(mState == States::Browsing)
		{
			drawList();
			drawItemInfo();
		}

		if(mState == States::Playing)
		{
			drawTrackInfo();
			drawPlayControls();
		}

		mNeedRefresh = false;
		mLCD->sendBuffer();
	}
}

void UI::drawList()
{
	for(uint8_t i = 0; i < UI_ITEM_PER_PAGE; i++)
	{
		mLCD->setCursor((10), char(10+10*i));
		uint8_t j = mCurrentPage*UI_ITEM_PER_PAGE+i;

		uint8_t SelectedItem = mCurrentItem+mCurrentPage*UI_ITEM_PER_PAGE;

		if(j < mList.size())
		{

			if(mNeedVScrolling && SelectedItem == j)
			{
				if(mCurrentVScroll > (Playlist.at(SelectedItem)->getTitle().length() - 20))
				{
					mCurrentVScroll = 0;
					mScrollTempo = 2;
				}

				mLCD->print(mList.at(j).substring(mCurrentVScroll));
			}

			else
				mLCD->print(mList.at(j));
		}
	}

	mLCD->setCursor(0, (mCurrentItem+1)*10);
	mLCD->print(">");
}

void UI::drawItemInfo()
{

	uint8_t SelectedItem = mCurrentItem+mCurrentPage*UI_ITEM_PER_PAGE;

	mLCD->drawHLine(0, 54, 128);

	mLCD->setFont(u8g2_font_4x6_tf);
	mLCD->setCursor(10, 62);
	mLCD->print("Durée : " + Playlist.at(SelectedItem)->getDurationS());
	mLCD->setFont(u8g2_font_helvR08_tf);

}

void UI::drawTrackInfo()
{
	mLCD->setCursor(2, 10);
	mLCD->print(mTrack->getTitle());

	mLCD->setFont(u8g2_font_4x6_tf);
	mLCD->setCursor(2, 37);
	mLCD->print(timeToString(mCurrentTime+1000*mTimeOffset));
	mLCD->setCursor(106, 37);
	mLCD->print(mTrack->getDurationS());

	mLCD->setFont(u8g2_font_helvR08_tf);

	mLCD->drawHLine(0, 39, 128);
	mLCD->drawBox(0, 40, uint8_t(128.0*(float(mCurrentTime+1000*mTimeOffset)/float(mTrack->getDuration()))), 5);
	mLCD->drawHLine(0, 45, 128);
}



void UI::drawPlayControls()
{

	mCurrentControl == Back ? mLCD->drawXBM(2, 48, 16, 16, minusTenS_bits) : mLCD->drawXBM(2, 48, 16, 16, minusTen_bits);

	mCurrentControl == VolM ? mLCD->drawXBM(20, 48, 16, 16, VolMoinsS_bits) : mLCD->drawXBM(20, 48, 16, 16, VolMoins_bits);
	mCurrentControl == VolP ? mLCD->drawXBM(92, 48, 16, 16, VolPlusS_bits) : mLCD->drawXBM(92, 48, 16, 16, VolPlus_bits);

	if(mPlayState == false)
	{
		if(mCurrentControl == PlayPause)
			mLCD->drawXBM(64, 48, 16, 16, PlayS_bits);
		else
			mLCD->drawXBM(64, 48, 16, 16, Play_bits);
	}

	else
	{
		if(mCurrentControl == PlayPause)
			mLCD->drawXBM(64, 48, 16, 16, PauseS_bits);
		else
			mLCD->drawXBM(64, 48, 16, 16, Pause_bits);
	}

	if(mCurrentControl == Stop)
		mLCD->drawXBM(48, 48, 16, 16, StopS_bits);
	else
		mLCD->drawXBM(48, 48, 16, 16, Stop_bits);

	mCurrentControl == Forw ? mLCD->drawXBM(110, 48, 16, 16, plusTenS_bits) : mLCD->drawXBM(110, 48, 16, 16, plusTen_bits);

}
