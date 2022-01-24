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

#include "CH376.h"

#define ANSWTIMEOUT 1000
#define MAXDIRDEPTH 3 // 3 = /subdir1/subdir2/subdir3

class CH376MSC : public CH376 {

public:
	CH376MSC(uint8_t spiSelect, uint8_t intPin, SPISettings speed = SPI_SCK_KHZ(125));
	CH376MSC(uint8_t spiSelect, SPISettings speed = SPI_SCK_KHZ(125)); //with SPI, MISO as INT pin(SPI bus can`t be shared with other SPI devices)
	virtual ~CH376MSC();

	uint8_t saveFileAttrb();
	uint8_t openFile();
	uint8_t closeFile();
	uint8_t moveCursor(uint32_t position);
	uint8_t deleteFile();
	uint8_t deleteDir();
	uint8_t listDir(const char* filename = "*");
	uint8_t readFile(char* buffer, uint8_t b_size = 0);
	uint8_t readRaw(uint8_t* buffer, uint8_t b_size = 0);
	int32_t readLong(char trmChar = '\n');
	uint32_t readULong(char trmChar = '\n');
	double readDouble(char trmChar = '\n');
	uint8_t writeChar(char trmChar);
	uint8_t writeFile(char* buffer, uint8_t b_size = 0);
	uint8_t writeRaw(uint8_t* buffer, uint8_t b_size = 0);
	uint8_t writeNum(uint8_t buffer);
	uint8_t writeNum(int8_t buffer);
	uint8_t writeNum(uint16_t buffer);
	uint8_t writeNum(int16_t buffer);
	uint8_t writeNum(uint32_t buffer);
	uint8_t writeNum(int32_t buffer);
	uint8_t writeNum(double buffer);
	uint8_t writeNumln(uint8_t buffer);
	uint8_t writeNumln(int8_t buffer);
	uint8_t writeNumln(uint16_t buffer);
	uint8_t writeNumln(int16_t buffer);
	uint8_t writeNumln(uint32_t buffer);
	uint8_t writeNumln(int32_t buffer);
	uint8_t writeNumln(double buffer);
	uint8_t cd(const char* dirPath, bool mkDir);
	bool readFileUntil(char trmChar, char* buffer, uint8_t b_size = 0);
	bool driveReady();
	bool checkIntMessage();
	void setFileName(const char* filename);
	bool getDeviceStatus();
	void resetFileList();

	//set/get
	uint32_t getFreeSectors();
	uint32_t getTotalSectors();
	uint32_t getFileSize();
	uint32_t getCursorPos();
	uint16_t getYear();
	uint16_t getMonth();
	uint16_t getDay();
	uint16_t getHour();
	uint16_t getMinute();
	uint16_t getSecond();
	uint8_t getStreamLen();
	uint8_t getStatus();
	uint8_t getFileSystem();
	uint8_t getFileAttrb();
	uint8_t getSource();
	char* getFileName();
	void sendFilename(const char* filename);
	char* getFileSizeStr();
	bool getEOF();
	void setYear(uint16_t year);
	void setMonth(uint16_t month);
	void setDay(uint16_t day);
	void setHour(uint16_t hour);
	void setMinute(uint16_t minute);
	void setSecond(uint16_t second);
	void setSource(uint8_t inpSource);

private:
	void driveAttach();
	void driveDetach();
	void setError(uint8_t errCode);
	uint8_t reqByteWrite(uint8_t a);
	uint8_t writeMachine(uint8_t* buffer, uint8_t b_size = 0);
	uint8_t writeDataFromBuff(uint8_t* buffer);
	uint8_t readDataToBuff(uint8_t* buffer, uint8_t b_size = 0);
	uint8_t readMachine(uint8_t* buffer, uint8_t b_size = 0);
	uint8_t dirCreate();

	void rdFatInfo();
	void writeFatData();
	void constructDate(uint16_t value, uint8_t ymd);
	void constructTime(uint16_t value, uint8_t hms);
	void rstFileContainer();
	void rstDriveContainer();

	///////Internal Variables///////////////////////////////
	uint8_t _streamLength = 0;
	uint8_t _fileWrite = 0; // read or write mode, needed for close operation
	uint8_t _dirDepth = 0;// Don't check SD card if it's in subdir
	uint8_t _byteCounter = 0; //vital variable for proper reading,writing
	uint8_t _driveSource = 0;//0 = USB, 1 = SD
	uint16_t _sectorCounter = 0;// variable for proper reading
	uint8_t _answer = 0;

	char _filename[12];

	fileProcessENUM fileProcesSTM = REQUEST;
};