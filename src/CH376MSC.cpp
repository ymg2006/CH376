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

#include "CH376MSC.h"

CH376MSC::CH376MSC(uint8_t spiSelect, uint8_t intPin, SPISettings speed) : CH376(spiSelect, intPin, speed) {}
CH376MSC::CH376MSC(uint8_t spiSelect, SPISettings speed) : CH376(spiSelect, speed) {}
CH376MSC::~CH376MSC() {
	//  Auto-generated destructor stub
}

bool CH376MSC::driveReady() {//returns TRUE if the drive ready
	uint8_t tmpReturn = 0;
	if (_driveSource == 1) {//if SD
		if (!_dirDepth) {// just check SD card if it's in root dir
			setMode(MODE_HOST_0);//reinit otherwise is not possible to detect if the SD card is removed
			setMode(MODE_HOST_SD);
			if (diskMount() == USB_INT_SUCCESS) {
				if (diskQuery(true) == USB_INT_DISK_READ) {
					_deviceAttached = true;
				}
			}
			else {
				driveDetach(); // do reinit otherwise mount will return always "drive is present"
			}//end if INT_SUCCESS
		}
		else tmpReturn = USB_INT_SUCCESS;//end if not ROOT
	}
	else {//if USB
		if (diskMount() == USB_INT_SUCCESS) {
			if (diskQuery(true) == USB_INT_DISK_READ) {
				_deviceAttached = true;
			}
		}//end if not INT SUCCESS
	}//end if interface
	return _deviceAttached;
}

bool CH376MSC::checkIntMessage() {
	uint8_t tmpReturn = 0;
	bool intRequest = false;
	while (!digitalRead(_intPin)) {
		tmpReturn = getInterrupt();
		delay(10);
	}
	switch (tmpReturn) {
	case USB_INT_CONNECT:
		intRequest = true;
		if (!_deviceAttached) driveAttach();
		break;
	case USB_INT_DISCONNECT:
		intRequest = true;
		if (_deviceAttached) driveDetach();
		break;
	}
	return intRequest;
}

void CH376MSC::setFileName(const char* filename){
	CH376::setFileName(filename);
}

uint8_t CH376MSC::openFile() {
	if (!_deviceAttached) return 0x00;
	return fileOpen(true);
}

uint8_t CH376MSC::saveFileAttrb() {
	uint8_t tmpReturn = 0;
	if (!_deviceAttached) return 0x00;
	_fileWrite = 1;

	dirInfoRead(0xff);
	writeFatData();//send fat data
	return dirInfoSave();
}

uint8_t CH376MSC::closeFile() { // 0x00 - w/o filesize update, 0x01 with filesize update
	uint8_t tmpReturn = 0;
	uint8_t d = 0x00;
	if (!_deviceAttached) return 0x00;

	if (_fileWrite == 1) { // if closing file after write procedure
		d = 0x01; // close with 0x01 (to update file length)
	}

	tmpReturn = fileClose(d);

	cd("/", 0);//back to the root directory if any file operation has occurred
	rstFileContainer();
	return tmpReturn;
}

uint8_t CH376MSC::deleteFile() {
	if (!_deviceAttached) return 0x00;
	openFile();
	_answer = fileErase();
	cd("/", 0);
	return _answer;
}

uint8_t CH376MSC::listDir(const char* filename) {
	/* __________________________________________________________________________________________________________
	 * | 00 - 07 | 08 - 0A |  	0B     |     0C    |     0D     | 0E  -  0F | 10  -  11 | 12 - 13|  14 - 15 |
	 * |Filename |Extension|File attrib|User attrib|First ch del|Create time|Create date|Owner ID|Acc rights|
	 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	 * | 16 - 17 | 18 - 19 |   1A - 1B   |  1C  -  1F |
	 * |Mod. time|Mod. date|Start cluster|  File size |
	 */
	bool moreFiles = true;  // more files waiting for read out
	bool doneFiles = false; // done with reading a file
	uint32_t tmOutCnt = millis();

	while (!doneFiles) {
		if (millis() - tmOutCnt >= ANSWTIMEOUT) setError(ERR_TIMEOUT);
		if (!_deviceAttached) {
			moreFiles = false;
			break;
		}
		switch (fileProcesSTM) {
		case REQUEST:
			setFileName(filename);
			_answer = openFile();
			//_fileWrite = 2; // if in subdir
			fileProcesSTM = READWRITE;
			break;
		case READWRITE:
			if (_answer == ERR_MISS_FILE) {
				fileProcesSTM = DONE;
				moreFiles = false;// no more files in the directory
			}//end if
			if (_answer == USB_INT_DISK_READ) {
				rdFatInfo(); // read data to fatInfo buffer
				fileProcesSTM = NEXT;
			}
			break;
		case NEXT:
			_answer = fileEnumGo(); // go for the next filename
			fileProcesSTM = DONE;
			break;
		case DONE:
			if (!moreFiles) {
				//closeFile(); // if no more files in the directory, close the file
				//closing file is not required after print dir (return value was always 0xB4 File is closed)
				fileProcesSTM = REQUEST;
			}
			else {
				fileProcesSTM = READWRITE;
			}
			doneFiles = true;
			break;
		}// end switch
	}//end while
	return moreFiles;
}

uint8_t CH376MSC::moveCursor(uint32_t position) {
	uint8_t tmpReturn = 0;
	if (!_deviceAttached) return 0x00;

	if (position > OpenDirInfo.DIR_FileSize) {	//fix for moveCursor issue #3 Sep 17, 2019
		_sectorCounter = OpenDirInfo.DIR_FileSize % DEF_SECTOR_SIZE;
	}
	else {
		_sectorCounter = position % DEF_SECTOR_SIZE;
	}
	CursorPos.mSectorLba = position;//temporary
	tmpReturn = movePointer(CursorPos.mByte[0], CursorPos.mByte[1], CursorPos.mByte[2], CursorPos.mByte[3]);

	if (CursorPos.mSectorLba > OpenDirInfo.DIR_FileSize) {
		CursorPos.mSectorLba = OpenDirInfo.DIR_FileSize;//set the valid position
	}
	return tmpReturn;
}

uint8_t CH376MSC::cd(const char* dirPath, bool mkDir) {
	uint8_t tmpReturn = 0;
	uint8_t pathLen = strlen(dirPath);
	if (!_deviceAttached) return 0x00;

	_dirDepth = 0;
	if (pathLen < ((MAXDIRDEPTH * 8) + (MAXDIRDEPTH + 1))) {//depth*(8char filename)+(directory separators)
		char input[pathLen + 1];
		strcpy(input, dirPath);
		setFileName("/");
		tmpReturn = openFile();
		char* command = strtok(input, "/");//split path into tokens
		while (command != NULL && !_errorCode) {
			if (strlen(command) > 8) {//if a dir name is longer than 8 char
				tmpReturn = ERR_LONGFILENAME;
				break;
			}
			setFileName(command);
			tmpReturn = openFile();
			if (tmpReturn == USB_INT_SUCCESS) {//if file already exist with this name
				tmpReturn = ERR_FOUND_NAME;
				closeFile();
				break;
			}
			else if (mkDir && (tmpReturn == ERR_MISS_FILE)) {
				tmpReturn = dirCreate();
				if (tmpReturn != USB_INT_SUCCESS) break;
			}//end if file exist
			_dirDepth++;
			command = strtok(NULL, "/");
		}
	}
	else {
		tmpReturn = ERR_LONGFILENAME;
	}//end if path is to long
	return tmpReturn;
}

uint8_t CH376MSC::deleteDir() {

	if (!_deviceAttached) return 0x00;
	_answer = fileErase();

	cd("/", 0);
	return _answer;
}

#pragma region Write
uint8_t CH376MSC::writeFile(char* buffer, uint8_t b_size) {
	return writeMachine((uint8_t*)buffer, b_size);
}

uint8_t CH376MSC::writeRaw(uint8_t* buffer, uint8_t b_size) {
	if (b_size == 0) b_size = sizeof(buffer);
	return writeMachine(buffer, b_size);
}

uint8_t CH376MSC::writeChar(char trmChar) {
	return writeMachine((uint8_t*)&trmChar, 1);
}

uint8_t CH376MSC::writeNum(uint8_t buffer) {
	char strBuffer[4];//max 255 = 3+1 char
	itoa(buffer, strBuffer, 10);
	return writeMachine((uint8_t*)strBuffer, strlen(strBuffer));
}

uint8_t CH376MSC::writeNum(int8_t buffer) {
	char strBuffer[5];//max -128 = 4+1 char
	itoa(buffer, strBuffer, 10);
	return writeMachine((uint8_t*)strBuffer, strlen(strBuffer));
}

uint8_t CH376MSC::writeNumln(uint8_t buffer) {
	char strBuffer[6];//max 255 = 3+2+1 char
	itoa(buffer, strBuffer, 10);
	strcat(strBuffer, "\r\n");
	return writeMachine((uint8_t*)strBuffer, strlen(strBuffer));
}

uint8_t CH376MSC::writeNumln(int8_t buffer) {
	char strBuffer[7];//max -128 = 4+2+1 char
	itoa(buffer, strBuffer, 10);
	strcat(strBuffer, "\r\n");
	return writeMachine((uint8_t*)strBuffer, strlen(strBuffer));
}

uint8_t CH376MSC::writeNum(uint16_t buffer) {
	char strBuffer[6];//max 65535 = 5+1 char
	itoa(buffer, strBuffer, 10);
	return writeMachine((uint8_t*)strBuffer, strlen(strBuffer));
}

uint8_t CH376MSC::writeNum(int16_t buffer) {
	char strBuffer[7];//max -32768 = 6+1 char
	itoa(buffer, strBuffer, 10);
	return writeMachine((uint8_t*)strBuffer, strlen(strBuffer));
}

uint8_t CH376MSC::writeNumln(uint16_t buffer) {
	char strBuffer[8];//max 65535 = 5+2+1 char
	itoa(buffer, strBuffer, 10);
	strcat(strBuffer, "\r\n");
	return writeMachine((uint8_t*)strBuffer, strlen(strBuffer));
}

uint8_t CH376MSC::writeNumln(int16_t buffer) {
	char strBuffer[9];//max -32768 = 6+2+1 char
	itoa(buffer, strBuffer, 10);
	strcat(strBuffer, "\r\n");
	return writeMachine((uint8_t*)strBuffer, strlen(strBuffer));
}

uint8_t CH376MSC::writeNum(uint32_t buffer) {
	char strBuffer[11];//max 4 294 967 295 = 10+1 char
	ltoa(buffer, strBuffer, 10);
	return writeMachine((uint8_t*)strBuffer, strlen(strBuffer));
}

uint8_t CH376MSC::writeNum(int32_t buffer) {
	char strBuffer[12];//max -2147483648 = 11+1 char
	ltoa(buffer, strBuffer, 10);
	return writeMachine((uint8_t*)strBuffer, strlen(strBuffer));
}

uint8_t CH376MSC::writeNumln(uint32_t buffer) {
	char strBuffer[13];//max 4 294 967 295 = 10+2+1 char
	ltoa(buffer, strBuffer, 10);
	strcat(strBuffer, "\r\n");
	return writeMachine((uint8_t*)strBuffer, strlen(strBuffer));
}

uint8_t CH376MSC::writeNumln(int32_t buffer) {
	char strBuffer[14];//max -2147483648 = 11+2+1 char
	ltoa(buffer, strBuffer, 10);
	strcat(strBuffer, "\r\n");
	return writeMachine((uint8_t*)strBuffer, strlen(strBuffer));
}

uint8_t CH376MSC::writeNum(double buffer) {
	char strBuffer[15];
	if (buffer > 4100000.00 || buffer < -4100000.00) {//beyond these values, the accuracy decreases rapidly
		strcpy(strBuffer, "ovf");					//constant determined by trial and error
	}
	else {
		dtostrf(buffer, 1, 2, strBuffer);
	}
	return writeMachine((uint8_t*)strBuffer, strlen(strBuffer));
}

uint8_t CH376MSC::writeNumln(double buffer) {
	char strBuffer[15];
	if (buffer > 4100000.00 || buffer < -4100000.00) {
		strcpy(strBuffer, "ovf");
	}
	else {
		dtostrf(buffer, 1, 2, strBuffer);
	}
	strcat(strBuffer, "\r\n");
	return writeMachine((uint8_t*)strBuffer, strlen(strBuffer));
}

uint8_t CH376MSC::writeDataFromBuff(uint8_t* buffer) {//====================
	uint8_t oldCounter = _byteCounter; //old buffer counter
	uint8_t dataLength; // data stream size

	dataLength = writeRequestedData(); // data stream size

	while (_byteCounter < (dataLength + oldCounter)) {
		spiWrite(buffer[_byteCounter]); // read data from buffer and write to serial port
		_byteCounter++;
	}//end while
	spiEndTransfer();
	return dataLength;
}

uint8_t CH376MSC::writeMachine(uint8_t* buffer, uint8_t b_size) {
	bool diskFree = true; //free space on a disk
	bool bufferFull = true; //continue to write while there is data in the temporary buffer
	uint32_t tmOutCnt = 0;
	if (!_deviceAttached) return 0x00;
	_fileWrite = 1; // read mode, required for close procedure
	_byteCounter = 0;

	if (DiskQueryInfo.mFreeSector == 0) {
		diskFree = false;
		return diskFree;
	}
	if (_answer == ERR_MISS_FILE) { // no file with given name
		_answer = fileCreate();
	}//end if CREATED

	if (_answer == ERR_FILE_CLOSE) {
		_answer = openFile();
	}

	if (_answer == USB_INT_SUCCESS) { // file created succesfully
		tmOutCnt = millis();
		while (bufferFull) {
			if (millis() - tmOutCnt >= ANSWTIMEOUT) setError(ERR_TIMEOUT);
			if (!_deviceAttached) {
				diskFree = false;
				break;
			}
			switch (fileProcesSTM) {
			case REQUEST:
				_answer = reqByteWrite(b_size - _byteCounter);

				if (_answer == USB_INT_SUCCESS) {
					fileProcesSTM = NEXT;

				}
				else if (_answer == USB_INT_DISK_WRITE) {
					fileProcesSTM = READWRITE;
				}
				break;
			case READWRITE:
				writeDataFromBuff(buffer);
				if (_byteCounter != b_size) {
					fileProcesSTM = NEXT;
				}
				else {
					fileProcesSTM = DONE;
				}
				break;
			case NEXT:
				if (DiskQueryInfo.mFreeSector > 0) {
					DiskQueryInfo.mFreeSector--;
					_answer = byteWriteGo();
					if (_answer == USB_INT_SUCCESS) {
						fileProcesSTM = REQUEST;
					}
					else if (_byteCounter != b_size) {
						fileProcesSTM = READWRITE;
					}
				}
				else { // if disk is full
					fileProcesSTM = DONE;
					diskFree = false;
				}
				break;
			case DONE:
				fileProcesSTM = REQUEST;
				CursorPos.mSectorLba += _byteCounter;
				_byteCounter = 0;
				_answer = byteWriteGo();
				bufferFull = false;
				break;
			}//end switch
		}//end while
	}// end file created

	return diskFree;
}
#pragma endregion

#pragma region Read
bool CH376MSC::readFileUntil(char trmChar, char* buffer, uint8_t b_size) {
	if (b_size == 0) b_size = sizeof(buffer);
	char tmpBuff[2];//temporary buffer to read string and analyze
	bool readMore = true;
	uint8_t charCnt = 0;
	b_size--;// last byte is reserved for NULL terminating character
	if (!_deviceAttached) return false;
	for (; charCnt < b_size; charCnt++) {
		readMore = readFile(tmpBuff, sizeof(tmpBuff));
		buffer[charCnt] = tmpBuff[0];
		if ((tmpBuff[0] == trmChar) || !readMore) {// reach terminate character or EOF
			readMore = false;
			break;
		}
	}
	buffer[charCnt + 1] = 0x00;//string terminate
	return readMore;
}

uint8_t CH376MSC::readFile(char* buffer, uint8_t b_size) {
	if (b_size == 0) b_size = sizeof(buffer);
	uint8_t tmpReturn;
	b_size--;// last byte is reserved for NULL terminating character
	tmpReturn = readMachine((uint8_t*)buffer, b_size);
	buffer[_byteCounter] = '\0';// NULL terminating char
	CursorPos.mSectorLba += _byteCounter;
	_streamLength = _byteCounter;
	_byteCounter = 0;
	return tmpReturn;
}

uint8_t CH376MSC::readRaw(uint8_t* buffer, uint8_t b_size) {
	if (b_size == 0) b_size = sizeof(buffer);
	uint8_t tmpReturn;
	tmpReturn = readMachine(buffer, b_size);
	CursorPos.mSectorLba += _byteCounter;
	_streamLength = _byteCounter;
	_byteCounter = 0; // make it 0 when the buffer is full
	return tmpReturn;
}

int32_t CH376MSC::readLong(char trmChar) {
	char workBuffer[18];
	int32_t retval;
	readFileUntil(trmChar, workBuffer, 14);
	retval = atol(workBuffer);
	return retval;
}

uint32_t CH376MSC::readULong(char trmChar) {
	char workBuffer[18];
	uint32_t retval;
	readFileUntil(trmChar, workBuffer, 14);
	retval = atol(workBuffer);
	return retval;
}

double CH376MSC::readDouble(char trmChar) {
	double retval;
	char workBuffer[18];
	readFileUntil(trmChar, workBuffer, 14);
	retval = atof(workBuffer);
	return retval;
}

uint8_t CH376MSC::readDataToBuff(uint8_t* buffer, uint8_t b_size) {
	if (b_size == 0) b_size = sizeof(buffer);
	uint8_t oldCounter = _byteCounter; //old buffer counter
	uint8_t dataLength = 0; // data stream size

	dataLength = readUSBData0(); // data stream size
	if (dataLength > b_size) {
		setError(ERR_OVERFLOW);//overflow
		return 0;
	}
	while (_byteCounter < (dataLength + oldCounter)) {
		buffer[_byteCounter] = spiRead(); // incoming data add to buffer
		_byteCounter++;
	}//end while
	spiEndTransfer();

	return dataLength;
}

uint8_t CH376MSC::readMachine(uint8_t* buffer, uint8_t b_size) { //buffer for reading, buffer size
	if (b_size == 0) b_size = sizeof(buffer);
	uint8_t tmpReturn = 0;// more data
	uint8_t byteForRequest = 0;
	bool bufferFull = false;
	uint32_t tmOutCnt = 0;
	_fileWrite = 0; // read mode, required for close procedure
	if (_answer == ERR_FILE_CLOSE || _answer == ERR_MISS_FILE) {
		bufferFull = true;
		tmpReturn = 0;// we have reached the EOF
	}
	tmOutCnt = millis();

	while (!bufferFull) {
		if (millis() - tmOutCnt >= ANSWTIMEOUT) setError(ERR_TIMEOUT);
		if (!_deviceAttached) {
			tmpReturn = 0;
			break;
		}
		switch (fileProcesSTM) {
		case REQUEST:
			byteForRequest = b_size - _byteCounter;
			if (_sectorCounter == DEF_SECTOR_SIZE) { //if one sector has read out
				_sectorCounter = 0;
				fileProcesSTM = NEXT;
				break;
			}
			else if ((_sectorCounter + byteForRequest) > DEF_SECTOR_SIZE) {
				byteForRequest = DEF_SECTOR_SIZE - _sectorCounter;
			}
			////////////////
			_answer = readByte(byteForRequest);
			if (_answer == USB_INT_DISK_READ) {
				fileProcesSTM = READWRITE;
				tmpReturn = 1; //we have not reached the EOF
			}
			else if (_answer == USB_INT_SUCCESS) { // no more data, EOF
				fileProcesSTM = DONE;
				tmpReturn = 0;
			}
			break;
		case READWRITE:
			_sectorCounter += readDataToBuff(buffer, byteForRequest);	//fillup the buffer
			if (_byteCounter != b_size) {
				fileProcesSTM = REQUEST;
			}
			else {
				fileProcesSTM = DONE;
			}
			break;
		case NEXT:
			_answer = byteReadGo();
			fileProcesSTM = REQUEST;
			break;
		case DONE:
			fileProcesSTM = REQUEST;
			bufferFull = true;
			break;
		}//end switch
	}//end while
	return tmpReturn;
}
#pragma endregion

#pragma region API
void CH376MSC::writeFatData() {// see fat info table under next filename
	uint8_t fatInfBuffer[32]; //temporary buffer for raw file FAT info
	memcpy(&fatInfBuffer, &OpenDirInfo, sizeof(fatInfBuffer)); //copy raw data to temporary buffer
	writeOffsetData((uint8_t)0x00, 32);
	for (uint8_t d = 0; d < 32; d++) {
		spiWrite(fatInfBuffer[d]);
	}
	spiEndTransfer();
}

void CH376MSC::rdFatInfo() {
	uint8_t fatInfBuffer[32]; //temporary buffer for raw file FAT info
	uint8_t dataLength;
	bool owrflow = false;

	dataLength = readUSBData0();
	if (dataLength > sizeof(fatInfBuffer)) {
		owrflow = true;
		dataLength = sizeof(fatInfBuffer);
	}
	spiReadMultiple(fatInfBuffer, dataLength);
	spiEndTransfer();

	if (owrflow) {
		setError(ERR_OVERFLOW);
	}
	else {
		memcpy(&OpenDirInfo, &fatInfBuffer, sizeof(fatInfBuffer)); //copy raw data to structured variable
	}
}

uint8_t CH376MSC::reqByteWrite(uint8_t a) {
	uint8_t tmpReturn = 0;
	tmpReturn = writeByte(a, (uint8_t)0x00);

	if (!_errorCode && (tmpReturn != USB_INT_SUCCESS) && (tmpReturn != USB_INT_DISK_WRITE)) {
		setError(tmpReturn);
	}
	return tmpReturn;
}

uint8_t CH376MSC::dirCreate() {
	return dirCreate();
}
#pragma endregion

#pragma region SetGet
void CH376MSC::setSource(uint8_t inpSource) {
	if (_driveSource != inpSource) {
		_driveSource = inpSource;
		if (_driveSource == 1) {// SD mode
			driveDetach();
			setMode(MODE_DEFAULT);
			setMode(MODE_HOST_SD);
			driveAttach();
		}
		else {// USB mode
			driveDetach();
			driveAttach();
		}//end if SD
	}//end if not
}

uint8_t CH376MSC::getSource() {
	return _driveSource;
}

uint8_t CH376MSC::getFileAttrb() {
	return OpenDirInfo.DIR_Attr;
}

char* CH376MSC::getFileName() {
	strncpy(_filename, OpenDirInfo.DIR_Name, 11);
	_filename[11] = '\0';
	return _filename;
}

void CH376MSC::sendFilename(const char* filename) {
	if (_deviceAttached) {
		if (strlen(filename)) { //copy if file name is given
			strncpy(_filename, filename, 12); //copy the filename string to internal filename variable
		}
		else {
			getFileName();
		}
		setFileName(_filename); // send to the CH376MSC
	}
}

uint8_t CH376MSC::getStatus() {
	return _answer;
}

uint32_t CH376MSC::getFileSize() {
	return OpenDirInfo.DIR_FileSize;
}

uint32_t CH376MSC::getCursorPos() {
	return CursorPos.mSectorLba;
}

char* CH376MSC::getFileSizeStr() { // make formatted file size string from unsigned long
	// final string is declared as static, return value
	static char fsizeString[10];// e.g 1023 byte\0 , 9 char long + NULL terminating char
	fsizeString[0] = 0;						  // or 1023,9 Kb\0
	uint32_t ul_size = OpenDirInfo.DIR_FileSize;
	float fl_size;
	char strNumber[7]; // e.g 1023,9\0 , temporary buffer
	if (_deviceAttached) {
		if (ul_size >= 1048576) { // if the filesize is 1Mb or bigger
			fl_size = ul_size / 1048576.0;
			dtostrf(fl_size, 1, 1, fsizeString);//convert float to string
			strcat(fsizeString, " Mb");  // concatenate unit symbol
		}
		else if (ul_size >= 1024) { // if the filesize is in Kb range
			fl_size = ul_size / 1024.0;
			dtostrf(fl_size, 1, 1, fsizeString);//convert float to string
			strcat(fsizeString, " Kb");
		}
		else { // if the filesize is in byte range
			ltoa(ul_size, strNumber, 10);// convert long to string
			strcpy(fsizeString, strNumber);// copy to the final string
			strcat(fsizeString, " byte");// concatenate unit symbol
		}//end size
	}//end if attached

	return fsizeString; //return the final string
}

void CH376MSC::setYear(uint16_t year) { //Year(0 = 1980, 119 = 2099 supported under DOS/Windows, theoretically up to 127 = 2107)
	if (year > 2099) year = 2099;
	if (year < 1980) year = 1980;
	year -= 1980;
	constructDate(year, 0);
}

uint16_t CH376MSC::getYear() {
	uint16_t year = OpenDirInfo.DIR_WrtDate;
	year = year >> 9;
	year += 1980;
	return year;
}

void CH376MSC::setMonth(uint16_t month) {
	if (month > 12) month = 12;
	if (month < 1) month = 1;
	constructDate(month, 1);
}

uint16_t CH376MSC::getMonth() {
	uint16_t month = OpenDirInfo.DIR_WrtTime;
	month = month << 7;
	month = month >> 12;
	return month;
}

void CH376MSC::setDay(uint16_t day) {
	if (day > 31) day = 31;
	if (day < 1) day = 1;
	constructDate(day, 2);
}

uint16_t CH376MSC::getDay() {
	uint16_t day = OpenDirInfo.DIR_WrtDate;
	day = day << 11;
	day = day >> 11;
	return day;
}

void CH376MSC::setHour(uint16_t hour) { //Coordinated Universal Time (UTC)
	if (hour > 23) hour = 23;
	constructTime(hour, 0);
}

uint16_t CH376MSC::getHour() {
	uint16_t hour = OpenDirInfo.DIR_WrtTime;
	hour = hour >> 11;
	return hour;
}

void CH376MSC::setMinute(uint16_t minute) {
	if (minute > 59) minute = 59;
	constructTime(minute, 1);
}

uint16_t CH376MSC::getMinute() {
	uint16_t minute = OpenDirInfo.DIR_WrtTime;
	minute = minute << 5;
	minute = minute >> 10;
	return minute;
}

void CH376MSC::setSecond(uint16_t second) { //! 0-58 2sec steps
	if (second > 59) second = 59;
	second /= 2;
	constructTime(second, 2);
}

uint16_t CH376MSC::getSecond() {
	uint16_t second = OpenDirInfo.DIR_WrtTime;
	second = second << 11;
	second = second >> 11;
	second *= 2;
	return second;
}

void CH376MSC::constructDate(uint16_t value, uint8_t ymd) { // 0-year, 1-month, 2-day
	uint16_t tmpInt = OpenDirInfo.DIR_WrtDate;
	uint16_t year;
	uint16_t month;
	uint16_t day;
	/*<------- 0x19 --------> <------- 0x18 -------->
	 *15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
	 *y  y  y  y  y  y  y  m  m  m  m  d  d  d  d  d
	 */
	if (_deviceAttached) {
		year = tmpInt >> 9;
		year = year << 9;

		month = tmpInt << 7;
		month = month >> 12;
		month = month << 5;

		day = tmpInt << 11;
		day = day >> 11;

		switch (ymd) {
		case 0://year
			year = value;
			year = year << 9;
			break;
		case 1://month
			month = value;
			month = month << 5;
			break;
		case 2://day
			day = value;
			break;
		default:
			break;
		}//end switch
		OpenDirInfo.DIR_WrtDate = year + month + day;
	}//end if attached
}

void CH376MSC::constructTime(uint16_t value, uint8_t hms) {
	uint16_t tmpInt = OpenDirInfo.DIR_WrtTime;
	uint16_t hour;
	uint16_t minute;
	uint16_t second;
	/*<------- 0x17 --------> <------- 0x16 -------->
	 *15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
	 *h  h  h  h  h  m  m  m  m  m  m  s  s  s  s  s
	 */
	if (_deviceAttached) {
		hour = tmpInt >> 11; //cut minute and second
		hour = hour << 11; // set hour

		minute = tmpInt << 5; // cut hour
		minute = minute >> 10;// cut seconds
		minute = minute << 5; // set minute

		second = tmpInt << 11; // cut hour and minute
		second = second >> 11; // set second

		switch (hms) {
		case 0://hour
			hour = value;
			hour = hour << 11;
			break;
		case 1://minute
			minute = value;
			minute = minute << 5;
			break;
		case 2://second
			second = value;
			break;
		default:
			break;
		}//end switch
		OpenDirInfo.DIR_WrtTime = hour + minute + second;
	}//end if attached
}

uint32_t CH376MSC::getTotalSectors() { // disk(partition?) size in bytes = totalSector * DEF_SECTOR_SIZE
	return DiskQueryInfo.mTotalSector;
}

uint32_t CH376MSC::getFreeSectors() { // total free bytes = freeSector * DEF_SECTOR_SIZE
	return DiskQueryInfo.mFreeSector;
}

uint8_t CH376MSC::getFileSystem() { //0x01-FAT12, 0x02-FAT16, 0x03-FAT32
	return DiskQueryInfo.mDiskFat;
}

void CH376MSC::rstDriveContainer() {
	memset(&DiskQueryInfo, 0, sizeof(DiskQueryInfo));// fill up with NULL disk data container
}

void CH376MSC::rstFileContainer() {
	memset(&OpenDirInfo, 0, sizeof(OpenDirInfo));// fill up with NULL file data container
	_filename[0] = '\0'; // put  NULL char at the first place in a name string
	_fileWrite = 0;
	_sectorCounter = 0;
	CursorPos.mSectorLba = 0;
	_streamLength = 0;
}

void CH376MSC::resetFileList() {
	fileProcesSTM = REQUEST;
}

bool CH376MSC::getEOF() {
	if (CursorPos.mSectorLba < OpenDirInfo.DIR_FileSize) {
		return false;
	}
	else {
		return true;
	}
}

void CH376MSC::driveAttach() {
	uint8_t tmpReturn = 0;
	if (_driveSource == 0) {//if USB
		setMode(MODE_HOST_1);
		setMode(MODE_HOST_2);
		tmpReturn = waitInterrupt(true);
	}//end if usb
	if (tmpReturn == USB_INT_CONNECT) {
		for (uint8_t a = 0; a < 5; a++) { //try to mount, delay in worst case ~(number of attempts * ANSWTIMEOUT ms)
			tmpReturn = diskMount();
			if (tmpReturn == USB_INT_SUCCESS) {
				clearError();
				_deviceAttached = true;
				break;
			}
			else if (_errorCode != ERR_TIMEOUT) {
				break;
			}//end if Success
		}//end for
	}
	else driveDetach();
	if (_deviceAttached) diskQuery(true);
}

void CH376MSC::driveDetach() {
	if (_driveSource == 0) {//if USB
		setMode(MODE_HOST_0);
	}
	_deviceAttached = false;
	rstDriveContainer();
	rstFileContainer();
}

bool CH376MSC::getDeviceStatus(){
	return CH376::_deviceAttached;
}

uint8_t CH376MSC::getStreamLen() {
	return _streamLength;
}

void CH376MSC::setError(uint8_t errCode) {
	CH376::setError(errCode);
	_dirDepth = 0;
	_byteCounter = 0;
	_answer = 0;
	resetFileList();
	rstDriveContainer();
	rstFileContainer();
#ifdef DEBUG
	Serial.println();
	Serial.print("Error:");
	Serial.print("\t 0x");
	Serial.print(errCode, HEX);
	Serial.println();
#endif
}
#pragma endregion
