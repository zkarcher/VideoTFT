#include <SD.h>
#include <SPI.h>
//#include "ILI9341_t3.h"
#include "ILI9341_t3DMA.h"
#include "title.h"

#define VIDEO_BIN  "VIDEO.BIN"

// Use these with the Teensy 3.5 & 3.6 SD card
#define SDCARD_CS_PIN    BUILTIN_SDCARD
//#define SDCARD_MOSI_PIN  11  // not actually used
//#define SDCARD_SCK_PIN   13  // not actually used

#define V_WIDTH    (320)
#define V_HEIGHT   (180)
#define BITS_PP    (16)
#define BYTES_PP   (BITS_PP / 8)

#define TFT_DC      9 // 15
#define TFT_CS      10
#define TFT_RST     4  // 255 = unused, connect to 3.3V
#define TFT_MOSI    11
#define TFT_SCLK    13
#define TFT_MISO    12

const uint16_t PAL_COLORS = 256;	// of 2 bytes each: RGB565
const uint16_t FRAME_BYTES = (PAL_COLORS * 2) + (V_WIDTH * V_HEIGHT);
//const uint16_t FRAME_COUNT = 30 * 5;	// 30fps, 10 seconds?
//const uint32_t TOTAL_BYTES = FRAME_BYTES * FRAME_COUNT;

//ILI9341_t3 tft_slow = ILI9341_t3(TFT_CS, TFT_DC);

ILI9341_t3DMA tft = ILI9341_t3DMA(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK, TFT_MISO);

Sd2Card card;
SdVolume volume;
File f1;
uint32_t frame_count;

uint8_t buffer[FRAME_BYTES];
uint16_t * buffer16 = (uint16_t *)(&buffer[0]);

void setup() {
  // wait for the Arduino Serial Monitor to open
	Serial.begin(115200);
  while (!Serial) ;
  delay(50);

	bool status = initSDCard();
	if (!status) return;

	/*
	Serial.println("tft.begin();");
	tft.fillRect(0, 0, tft.width(), tft.height(), 0x001F);
	*/

	Serial.println("tft.begin();");
	tft.begin();

	// Start DMA mode
	tft.refresh();

	//tft.fillRect(0, 0, tft.width(), tft.height(), 0x001F);

	// Fill the buffer with red snow
	for (uint16_t x = 0; x < tft.width(); x++) {
		for (uint16_t y = 0; y < tft.height(); y++) {
			tft.ddrawPixel(x, y, random(0b11111) << 11);
		}
	}

	tft.dwriteRect8BPP(0, 0, 60, V_WIDTH, &title[PAL_COLORS * 2], (uint16_t *)title);

	/*
	delay(500);
	tft.stopRefresh();
	delay(500);
	*/

	Serial.println();
	Serial.println("Reading " VIDEO_BIN " :");

	// Read the header: A single uint32_t, contains number of frames
	uint8_t count[4];
	f1.read(count, 4);

	uint32_t * count32 = (uint32_t *)count;
	frame_count = *count32;
	Serial.print("frame_count: ");
	Serial.println(frame_count);
	delay(50);
}

void loop(void) {
	unsigned long start = millis();

	f1.seek(4);

	for (uint32_t f = 0; f < frame_count; f++) {
		// Read in 0xffff-byte chunks?
		f1.read(buffer, FRAME_BYTES);

		tft.wait();
		tft.dwriteRect8BPP(60, 0, V_HEIGHT, V_WIDTH, &buffer[PAL_COLORS * 2], buffer16);

		//tft.refreshOnce();	// Slow & flickery :(

		/*
		if ((f % 20) == 9) {
			tft.stopRefresh();
		} else if ((f % 20) == 19) {
			tft.refresh();
		}
		*/
	}

	unsigned long end = millis();

	Serial.print("  WIDTH:        ");
	Serial.println(tft.width());
	Serial.print("  HEIGHT:       ");
	Serial.println(tft.height());
	Serial.print("  V_WIDTH:      ");
	Serial.println(V_WIDTH);
	Serial.print("  V_HEIGHT:     ");
	Serial.println(V_HEIGHT);
	Serial.print("  FRAME_BYTES:  ");
	Serial.println(FRAME_BYTES);
	Serial.print("  frame_count:  ");
	Serial.println(frame_count);
	Serial.print("  millis total: ");
	Serial.println(end - start);
	Serial.print("  millis per frame: ");
	Serial.println((end - start) / frame_count);
	Serial.print("  FPS:            : ");
	Serial.print( frame_count / ((end - start) / 1000.0f) );

	Serial.println("      Done!\n");
}

void loop_test(void) {
	//void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
	uint16_t w = tft.width();
  uint16_t h = tft.height();

	uint16_t x0 = random(w);
	uint16_t x1 = random(w);
	uint16_t y0 = random(h);
	uint16_t y1 = random(h);
	uint16_t color = random(0xffff);

  tft.fillRect(min(x0,x1), min(y0,y1), abs(x0-x1), abs(y0-y1), color);

	delay(3000);
}


bool initSDCard()
{
	boolean status;
	int type;
	float size;

	// Configure SPI
	/*
	SPI.setMOSI(SDCARD_MOSI_PIN);
	SPI.setSCK(SDCARD_SCK_PIN);
	*/

	Serial.begin(115200);
	Serial.println("SD Card Test");
	Serial.println("------------");

	// First, detect the card
	status = card.init(SPI_FULL_SPEED, SDCARD_CS_PIN);
	if (status) {
		Serial.println("SD card is connected :-)");
	} else {
		Serial.println("SD card is not connected or unusable :-(");
		return false;
	}

	type = card.type();
	if (type == SD_CARD_TYPE_SD1 || type == SD_CARD_TYPE_SD2) {
		Serial.println("Card type is SD");
	} else if (type == SD_CARD_TYPE_SDHC) {
		Serial.println("Card type is SDHC");
	} else {
		Serial.println("Card is an unknown type (maybe SDXC?)");
	}

	// Then look at the file system and print its capacity
	status = volume.init(card);
	if (!status) {
		Serial.println("Unable to access the filesystem on this card. :-(");
		return false;
	}

	size = volume.blocksPerCluster() * volume.clusterCount();
	size = size * (512.0 / 1e6); // convert blocks to millions of bytes
	Serial.print("File system space is ");
	Serial.print(size);
	Serial.println(" Mbytes.");

	// Now open the SD card normally
	status = SD.begin(SDCARD_CS_PIN);
	if (status) {
		Serial.println("SD library is able to access the filesystem");
	} else {
		Serial.println("SD library can not access the filesystem!");
		Serial.println("Please report this problem, with the make & model of your SD card.");
		Serial.println("  http://forum.pjrc.com/forums/4-Suggestions-amp-Bug-Reports");
	}

	// Open the 4 sample files.  Hopefully they're on the card
	f1 = SD.open(VIDEO_BIN);

	// Speed test reading a single file
	if (!f1) {
		Serial.println("Unable to find " VIDEO_BIN " on this card");
		return false;
	}

	return true;
}
