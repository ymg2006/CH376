/*
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <Arduino.h>
#include <Stream.h>
#include <SPI.h>
#include "CH376DEF.h"

#define DEBUG

#if defined(__STM32F1__)
#include "itoa.h"
#endif

#if defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD)
#include "avr/dtostrf.h"
#endif

#define SPI_SCK_KHZ(speedKhz) SPISettings(1000UL * speedKhz, MSBFIRST, SPI_MODE0) //get the speed in KHz
#define SPI_SCK_MHZ(speedMhz) SPISettings(1000000UL * speedMhz, MSBFIRST, SPI_MODE0) //get the speed in MHz

class CH376 {
public:
	CH376(uint8_t spiSelect, uint8_t intPin, SPISettings speed = SPI_SCK_KHZ(125));
	CH376(uint8_t spiSelect, SPISettings speed = SPI_SCK_KHZ(125));
	virtual ~CH376();

	void init();
	bool pingDevice(byte value = 0x01);
	bool getDeviceAttached();
	bool getControllerReady();
	bool setMode(USB_MODE mode = MODE_HOST_0);
	void setSpeed(USB_SPEED speed);
	uint8_t getError();
protected:
	uint8_t waitInterrupt(bool endTransfer = true);
	void setError(uint8_t errCode);
	void clearError();

	void spiBeginTransfer();
	void spiEndTransfer();
	void spiWrite(uint8_t data);
	void spiPrint(const char str[]);
	uint8_t spiRead();
	uint8_t spiReadMultiple(uint8_t* buffer, uint8_t b_size);

	void exec00(uint8_t CMD00);
	void abortNAK();
	void dirtyBuffer();
	void enterSleep();
	void resetAll();
	void unlockUSB();

	void exec10(uint8_t CMD10, uint8_t input, bool endTransfer = true);
	void setENDPoint2(uint8_t input);
	void setENDPoint3(uint8_t input);
	void setENDPoint4(uint8_t input);
	void setENDPoint5(uint8_t input);
	void setENDPoint6(uint8_t input);
	void setENDPoint7(uint8_t input);
	void setFileName(uint8_t input);
	void setFileName(const char* filename = "");
	void setUSBAddress(uint8_t input);
	void setUSBSpeed(uint8_t input);
	void writeHostData(uint8_t input);
	void writeUSBData3(uint8_t input);
	void writeUSBData5(uint8_t input);
	void writeUSBData7(uint8_t input);

	void exec20(uint8_t CMD20, uint8_t input, uint8_t input2, bool endTransfer = true);
	void checkSuspended(uint8_t input, uint8_t input2);
	void setRetry(uint8_t input, uint8_t input2);
	void setSDOINT(uint8_t input, uint8_t input2);
	void writeVAR8(uint8_t input, uint8_t input2);
	void writeOffsetData(uint8_t input, uint8_t input2);

	void execx0(uint8_t CMDx0, uint8_t input[], int num, bool endTransfer = true);
	void setUSBID(uint8_t input, uint8_t input2, uint8_t input3, uint8_t input4);
	void setFileSize(uint8_t input, uint8_t input2, uint8_t input3, uint8_t input4, uint8_t input5);
	void writeVAR32(uint8_t input, uint8_t input2, uint8_t input3, uint8_t input4, uint8_t input5);

	uint8_t exec01(uint8_t CMD01, bool endTransfer = true);
	uint8_t delay100US();
	uint8_t getICVersion();
	uint8_t getInterrupt();
	uint8_t readUSBData();
	uint8_t readUSBData0();
	uint8_t testConnect();
	uint8_t writeRequestedData();

	uint8_t exec11(uint8_t CMD11, uint8_t input, bool endTransfer = true);
	uint8_t CheckExist(uint8_t input);
	USB_SPEED getDevRate(uint8_t input);
	uint8_t getToggle(uint8_t input);
	uint8_t readVar8(uint8_t input);
	uint8_t setUSBMode(uint8_t input);

	uint8_t exec21(uint8_t CMD21, uint8_t input, uint8_t input2, bool endTransfer = true);
	uint8_t setBaudrate(uint8_t input, uint8_t input2);

	uint8_t exec0H(uint8_t CMD0H, bool endTransfer = true);
	uint8_t autoSetup();
	uint8_t byteReadGo();
	uint8_t byteWriteGo();
	uint8_t dirCreate();
	uint8_t dirInfoSave();
	uint8_t diskBulgOnly(bool fillStruct = false);
	uint8_t diskCapacity();
	uint8_t diskConnect();
	uint8_t diskInit(bool fillStruct = false);
	uint8_t diskInquiry(bool fillStruct = false);
	uint8_t diskMaxLogicalUnitNumber();
	uint8_t diskMount(bool fillStruct = false);
	uint8_t diskQuery(bool fillStruct = false);
	uint8_t diskReadGo();
	uint8_t diskReady();
	uint8_t diskReset();
	uint8_t diskCheckErrors(bool fillStruct = false);
	uint8_t diskSize();
	uint8_t diskWriteGo();
	uint8_t fileCreate();
	uint8_t fileEnumGo(bool fillStruct = false);
	uint8_t fileErase();
	uint8_t fileOpen(bool fillStruct = false);
	uint8_t readDiskSector();
	uint8_t writeDiskSector();

	uint8_t exec1H(uint8_t CMD1H, uint8_t input, bool endTransfer = true);
	uint8_t clearEndPointError(uint8_t input);
	uint8_t dirInfoRead(uint8_t input);
	uint8_t fileClose(uint8_t input);
	uint8_t getDescription(uint8_t input);
	uint8_t issueToken(uint8_t input);
	uint8_t readFromSector(uint8_t input);
	uint8_t writeToSector(uint8_t input);
	uint8_t setAddress(uint8_t input);
	uint8_t setConfig(uint8_t input);

	uint8_t exec2H(uint8_t CMD2H, uint8_t input, uint8_t input2, bool endTransfer = true);
	uint8_t readByte(uint8_t input, uint8_t input2 = 0x00);
	uint8_t writeByte(uint8_t input, uint8_t input2);
	uint8_t issueToken(uint8_t input, uint8_t input2);

	uint8_t execxH(uint8_t CMDxH, uint8_t input[], int num, bool endTransfer = true);
	uint8_t movePointer(uint8_t input, uint8_t input2, uint8_t input3, uint8_t input4);
	uint8_t moveSectorPointer(uint8_t input, uint8_t input2, uint8_t input3, uint8_t input4);
	uint8_t diskRead(uint8_t input, uint8_t input2, uint8_t input3, uint8_t input4, uint8_t input5);
	uint8_t diskWrite(uint8_t input, uint8_t input2, uint8_t input3, uint8_t input4, uint8_t input5);

///////Internal Variables///////////////////////////////
	SPISettings _spiSpeed;
	uint8_t _spiChipSelect;
	uint8_t _intPin;

	bool _deviceAttached = false;
	bool _controllerReady = false;

	uint8_t _errorCode = 0;

	FAT_DIR_INFO OpenDirInfo;
	FAT_DIR_INFO EnumDirInfo;
	ByteLocate CursorPos;
	DiskQuery DiskQueryInfo;
	BULK_ONLY_CBW DiskBocCbw;
	INQUIRY_DATA DiskMountInq;
	INQUIRY_DATA DiskInitInq;
	INQUIRY_DATA DiskInqData;
	SENSE_DATA ReqSenseData;
};
