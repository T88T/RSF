#include "Arduino.h"
#include <U8g2lib.h>

#include "MusicFile.h"
#include "UI.h"
#include "Encoder.h"
#include <SdFat.h>
#include <vs1053_SdFat.h>
#include "MusicFile.h"
#include <Array.h>

#define DF_POWER	11
#define MOSI		51
#define MISO		50
#define SCLK		52
#define CS_LCD		16
#define CS_SD		53
#define BTN_ENC		18
#define ENC_A		20
#define ENC_B		21


#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}

U8G2_ST7920_128X64_F_SW_SPI lcd(U8G2_R0, 23, 17, CS_LCD, U8X8_PIN_NONE);
UI RSF_UI(&lcd);
Encoder Encoder(ENC_A, ENC_B, BTN_ENC, &RSF_UI);
SdFat SD;
vs1053 MP3player;
Array<MusicFile*, 50> Playlist;

void rotaryEncoderChange();

uint32_t displayTimer = 0;
uint32_t previousTime = 0;
uint32_t currentTime = 0;

void setup()
{
	Serial.begin(9600);
	Serial3.begin(9600);

	lcd.begin();
	lcd.setFont(u8g2_font_4x6_tf);
	lcd.clearBuffer();
	lcd.drawXBMP(28, 0, 69, 64, RSF_bits);
	lcd.sendBuffer();

	if(!SD.begin(SD_SEL, SPI_FULL_SPEED))
	{
		lcd.clearBuffer();
		lcd.drawXBMP(28, 0, 69, 64, RSF_bits);
		lcd.setCursor(10, 59);
		lcd.print("Erreur de carte SD...");
		lcd.sendBuffer();
		SD.initErrorHalt();
	}

	if(!SD.chdir("/"))
		SD.errorHalt("sd.chdir");

	uint8_t Status = MP3player.begin();

	if(Status != 0)
	{
		Serial.print(F("Error code: "));
		Serial.print(Status);
		Serial.println(F(" when trying to start MP3 player"));
	}

	MP3player.setVolume(uint16_t(0x0000));

	File32 file;
	File32 root;

	if (!root.open("/"))
		Serial.println("Unable to open root");

	//root.rewind();
	//root.rewindDirectory();

	char fileName[60] = {0};

	while(file.openNext(&root, O_RDONLY))
	{
		for(uint8_t i; i < 60; i++)
			fileName[i] = 0;

		file.getName(fileName, 60);
		/*Serial.print("\t");*/ Serial.println(fileName);

		if(String(fileName).indexOf(".mp3") != -1 && !file.isDir())
		{
			MusicFile* track = new MusicFile(&file);
			if(track->isValid())
			{
				Playlist.push_back(track);
				lcd.clearBuffer();
				lcd.drawXBMP(28, 0, 69, 64, RSF_bits);
				lcd.setCursor(10, 59);
				lcd.print("Fichiers : " + String(Playlist.size()));
				lcd.sendBuffer();
			}
		}

		file.close();
	}

	Serial.print("Found files: "); Serial.println(Playlist.size());
	RSF_UI.fillList(&Playlist);

	lcd.setFont(u8g2_font_helvR08_tf);

	attachInterrupt(digitalPinToInterrupt(ENC_A), rotaryEncoderChange, CHANGE);
	attachInterrupt(digitalPinToInterrupt(ENC_B), rotaryEncoderChange, CHANGE);
	attachInterrupt(digitalPinToInterrupt(BTN_ENC), rotaryEncoderChange, CHANGE);

	Serial.print("Free memory : "); Serial.println(freeMemory());

	previousTime = millis();
}

void loop()
{
	//Check for inputs
		/* Inputs are Interrupt driven */

	//Timing
	currentTime = millis();
	uint32_t dt = currentTime - previousTime;
	previousTime = currentTime;

	displayTimer += dt;

	//Update logic
	RSF_UI.update(dt);

	//Draw
	if(displayTimer >= 100)
	{
		RSF_UI.draw();
		displayTimer = 0;
	}
}

void rotaryEncoderChange()
{
	Encoder.update();
}

