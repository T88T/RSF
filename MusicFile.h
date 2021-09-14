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
		MusicFile(File32 *file);
		String getTitle();
		String getPath();
		uint32_t getDuration();
		String getDurationS();
		uint16_t getBitrate();

		bool isValid();

	private:
		bool readInfo(File32 *file = nullptr);

		bool readTAG(File32 *track, uint8_t offset, char* infobuffer);

		bool mIsValid;
		String mPath;
		uint32_t mFileSize;
		String mTitle;
		uint32_t mDuration;
		String mDurationString;

};

#endif /* MUSICFILE_H_ */
