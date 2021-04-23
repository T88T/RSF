/*
 * MusicFile.h
 *
 *  Created on: 5 mars 2021
 *      Author: TooT
 */

#ifndef MUSICFILE_H_
#define MUSICFILE_H_

#include "Arduino.h"
#include "SdFat.h"

#define TAG_TITLE              3
#define TAG_ARTIST            33
#define TAG_ALBUM             63

class MusicFile
{
	public:
		MusicFile(String path);
		String getTitle();
		String getPath();
		String getExtension();
		uint32_t getDuration();
		String getDurationS();
		uint16_t getBitrate();

		bool isValid();

	private:
		bool readInfo();

		bool readTAG(File32 *track, uint8_t offset, char* infobuffer);

		bool mIsValid;
		String mPath;
		uint16_t mBitrate;
		uint32_t mFileSize;
		String mTitle;
		String mExtension;
		uint32_t mDuration;
		String mDurationString;
};

#endif /* MUSICFILE_H_ */
