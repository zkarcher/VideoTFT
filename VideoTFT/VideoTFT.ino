#include <SD.h>
#include <SPI.h>
//#include "ILI9341_t3.h"
#include "ILI9341_t3DMA.h"

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
const uint16_t FRAME_COUNT = 30 * 5;	// 30fps, 10 seconds?
const uint32_t TOTAL_BYTES = FRAME_BYTES * FRAME_COUNT;

//ILI9341_t3 tft_slow = ILI9341_t3(TFT_CS, TFT_DC);

ILI9341_t3DMA tft = ILI9341_t3DMA(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK, TFT_MISO);

Sd2Card card;
SdVolume volume;
File f1;

uint8_t buffer[FRAME_BYTES];
uint16_t * buffer16 = (uint16_t *)(&buffer[0]);

void setup() {
	// Fill the buffer with red snow
	for (uint16_t x = 0; x < V_WIDTH; x++) {
		for (uint16_t y = 0; y < V_HEIGHT; y++) {
			tft.drawPixel(x, y, random(0b11111) << 11);
		}
	}

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

	tft.fillRect(0, 0, tft.width(), tft.height(), 0x001F);

	Serial.println();
	Serial.println("Reading SDTEST1.WAV:");

	if (f1.size() < TOTAL_BYTES) {
		Serial.println("f1 is too small for speed testing");
		return;
	}

	unsigned long start = millis();

	//uint8_t * screen8 = tft.getScreen8();

	for (uint16_t f = 0; f < FRAME_COUNT; f++) {
		// Read in 0xffff-byte chunks?
		f1.read(buffer, FRAME_BYTES);

		tft.writeRect8BPP(0, 0, V_WIDTH, V_HEIGHT, &buffer[PAL_COLORS * 2], buffer16);

		//f1.read(buffer, FRAME_BYTES);

		// Blast this data onto the screen
		//tft.writeRect(0, 0, V_WIDTH, V_HEIGHT, buffer16);
		//tft.writeRect8BPP(0, 0, V_WIDTH, V_HEIGHT, &buffer[512], buffer16);
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
	Serial.print("  FRAME_COUNT:  ");
	Serial.println(FRAME_COUNT);
	Serial.print("  TOTAL_BYTES:  ");
	Serial.println(TOTAL_BYTES);
	Serial.print("  millis total: ");
	Serial.println(end - start);
	Serial.print("  millis per frame: ");
	Serial.println((end - start) / FRAME_COUNT);
	Serial.print("  FPS:            : ");
	Serial.print( FRAME_COUNT / ((end - start) / 1000.0f) );

	Serial.println("\nDone!");
}


void loop(void) {
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
	f1 = SD.open("SDTEST1.WAV");

	// Speed test reading a single file
	if (!f1) {
		Serial.println("Unable to find SDTEST1.WAV on this card");
		return false;
	}

	return true;
}
