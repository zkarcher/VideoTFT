#include <SD.h>
#include <SPI.h>

// Use these with the Teensy 3.5 & 3.6 SD card
#define SDCARD_CS_PIN    BUILTIN_SDCARD
#define SDCARD_MOSI_PIN  11  // not actually used
#define SDCARD_SCK_PIN   13  // not actually used

#define WIDTH    (320)
#define HEIGHT   (240)

Sd2Card card;
SdVolume volume;
File f1;
char buffer[WIDTH * HEIGHT];

void setup() {
  // wait for the Arduino Serial Monitor to open
  while (!Serial) ;
  delay(50);

	bool status = initSDCard();
	if (!status) return;

	Serial.println();
	Serial.println("Reading SDTEST1.WAV:");

	const uint32_t FRAME_SZ = WIDTH * HEIGHT;
	const uint16_t FRAME_COUNT = 30 * 10;	// 30fps, 10 seconds?
	const uint32_t TOTAL_BYTES = FRAME_SZ * FRAME_COUNT;

	if (f1.size() < TOTAL_BYTES) {
		Serial.println("f1 is too small for speed testing");
		return;
	}

	unsigned long start = millis();

	for (uint16_t f = 0; f < FRAME_COUNT; f++) {
		f1.read(buffer, FRAME_SZ);
	}

	unsigned long end = millis();

	Serial.print("  FRAME_COUNT: ");
	Serial.println(FRAME_COUNT);
	Serial.print("  FRAME_SZ:    ");
	Serial.println(FRAME_SZ);
	Serial.print("  TOTAL_BYTES: ");
	Serial.println(TOTAL_BYTES);
	Serial.print("  millis per frame: ");
	Serial.println((end - start) / FRAME_COUNT);

	Serial.println("\nDone!");
}


void loop(void) {
  // do nothing after the test
}


bool initSDCard()
{
	boolean status;
	int type;
	float size;

	// Configure SPI
	SPI.setMOSI(SDCARD_MOSI_PIN);
	SPI.setSCK(SDCARD_SCK_PIN);

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