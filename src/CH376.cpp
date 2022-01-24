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

CH376::CH376(uint8_t spiSelect, uint8_t intPin, SPISettings speed) {
	_intPin = intPin;
	_spiChipSelect = spiSelect;
	_spiSpeed = speed;
}
CH376::CH376(uint8_t spiSelect, SPISettings speed) {
	_intPin = MISO;
	_spiChipSelect = spiSelect;
	_spiSpeed = speed;
}
CH376::~CH376() {
	//  Auto-generated destructor stub
}

void CH376::init() {
	delay(60);
	if (_intPin != MISO) {
		pinMode(_intPin, INPUT_PULLUP);
	}
	pinMode(_spiChipSelect, OUTPUT);
	digitalWrite(_spiChipSelect, HIGH);
	SPI.begin();
	resetAll();
	delay(100);
	if (_intPin == MISO) {
		setSDOINT(0x16, 0x90); //10H=DISABLE SDO PIN FOR INTERRUPT OUTPUT
	}
	_controllerReady = pingDevice();
	setMode();
}
bool CH376::pingDevice(byte value) { return (CheckExist(value) == (255 - value)); }
bool CH376::getDeviceAttached() { return _deviceAttached; }
bool CH376::getControllerReady() { return _controllerReady; }
bool CH376::setMode(USB_MODE mode) { return (setUSBMode(mode) == CMD_RET_SUCCESS); }
void CH376::setSpeed(USB_SPEED speed) { setUSBSpeed(speed); }
void CH376::setError(uint8_t errCode) {
	_errorCode = errCode;
	_deviceAttached = false;
#ifdef DEBUG
	Serial.println();
	Serial.print("Error:");
	Serial.print("\t 0x");
	Serial.print(errCode, HEX);
	Serial.println();
#endif
}
uint8_t CH376::getError() { return _errorCode; }
void CH376::clearError() { _errorCode = 0; }

#pragma region SPI
void CH376::spiBeginTransfer() {
	delayMicroseconds(2);
	SPI.beginTransaction(_spiSpeed);
	digitalWrite(_spiChipSelect, LOW);
}
void CH376::spiEndTransfer() {
	digitalWrite(_spiChipSelect, HIGH);
	SPI.endTransaction();
}
void CH376::spiWrite(uint8_t data) {
	delayMicroseconds(2); // datasheet TSC min 1.5uSec
	SPI.transfer(data);
}
void CH376::spiPrint(const char str[]) {
	uint8_t stringCounter = 0;
	while (str[stringCounter]) {
		spiWrite(str[stringCounter]);
		stringCounter++;
	}
}
uint8_t CH376::spiRead() {
	delayMicroseconds(2); //datasheet TSC min 1.5uSec
	return SPI.transfer(0x00);
}
uint8_t CH376::spiReadMultiple(uint8_t* buffer, uint8_t b_size)
{
	if (b_size == 0) b_size = sizeof(buffer);
	int i;
	for (int i = 0; i < b_size; i++)
	{
		uint8_t value;
		uint8_t bytes = spiRead();
		*(buffer + i) = value;
	}
	return i;
}
uint8_t CH376::waitInterrupt(bool endTransfer) {
	uint32_t oldMillis = millis();
	while (digitalRead(_intPin)) {
		if ((millis() - oldMillis) > ANSWTIMEOUT) {
			setError(ERR_TIMEOUT);
			return 0x00;
		}
	}
	return getInterrupt();
}
#pragma endregion

#pragma region CMD00
void CH376::exec00(uint8_t CMD00) {
	spiBeginTransfer();
	spiWrite(CMD00);
	spiEndTransfer();
#ifdef DEBUG
	Serial.println();
	Serial.print("Command:");
	Serial.print("\t 0x");
	Serial.print(CMD00, HEX);
	Serial.println();
#endif
}

void CH376::abortNAK() { exec00(CMD00_ABORT_NAK); }
void CH376::dirtyBuffer() { exec00(CMD00_DIRTY_BUFFER); }
void CH376::enterSleep() { exec00(CMD00_ENTER_SLEEP); }
void CH376::resetAll() { exec00(CMD00_RESET_ALL); }
void CH376::unlockUSB() { exec00(CMD00_UNLOCK_USB); }
#pragma endregion

#pragma region CMD10
void CH376::exec10(uint8_t CMD10, uint8_t input, bool endTransfer) {
	spiBeginTransfer();
	spiWrite(CMD10);
	spiWrite(input);
	if (endTransfer) { spiEndTransfer(); }
#ifdef DEBUG
	Serial.println();
	Serial.print("Command:");
	Serial.print("\t 0x");
	Serial.print(CMD10, HEX);
	Serial.println();
	Serial.print("input:");
	Serial.print("\t 0x");
	Serial.print(input, HEX);
	Serial.println();
#endif // DEBUG
}

void CH376::setENDPoint2(uint8_t input) { exec10(CMD10_SET_ENDP2, input); }
void CH376::setENDPoint3(uint8_t input) { exec10(CMD10_SET_ENDP3, input); }
void CH376::setENDPoint4(uint8_t input) { exec10(CMD10_SET_ENDP4, input); }
void CH376::setENDPoint5(uint8_t input) { exec10(CMD10_SET_ENDP5, input); }
void CH376::setENDPoint6(uint8_t input) { exec10(CMD10_SET_ENDP6, input); }
void CH376::setENDPoint7(uint8_t input) { exec10(CMD10_SET_ENDP7, input); }
void CH376::setFileName(uint8_t input) { exec10(CMD10_SET_FILE_NAME, input); }
void CH376::setFileName(const char* filename) {
	spiBeginTransfer();
	spiWrite(CMD10_SET_FILE_NAME);
	//write(0x2f); // "/" root directory
	spiPrint(filename); // filename
	//write(0x5C);	// this is the "\" sign 
	spiWrite((uint8_t)0x00);	// terminating null character
	spiEndTransfer();
}
void CH376::setUSBAddress(uint8_t input) { exec10(CMD10_SET_USB_ADDR, input); }
void CH376::setUSBSpeed(uint8_t input) { exec10(CMD10_SET_USB_SPEED, input); }
void CH376::writeHostData(uint8_t input) { exec10(CMD10_WR_HOST_DATA, input, false); }
void CH376::writeUSBData3(uint8_t input) { exec10(CMD10_WR_USB_DATA3, input, false); }
void CH376::writeUSBData5(uint8_t input) { exec10(CMD10_WR_USB_DATA5, input, false); }
void CH376::writeUSBData7(uint8_t input) { exec10(CMD10_WR_USB_DATA7, input, false); }
#pragma endregion

#pragma region CMD20
void CH376::exec20(uint8_t CMD20, uint8_t input, uint8_t input2, bool endTransfer) {
	spiBeginTransfer();
	spiWrite(CMD20);
	spiWrite(input);
	spiWrite(input2);
	if (endTransfer) { spiEndTransfer(); }
#ifdef DEBUG
	Serial.println();
	Serial.print("Command:");
	Serial.print("\t 0x");
	Serial.print(CMD20, HEX);
	Serial.println();
	Serial.print("input:");
	Serial.print("\t 0x");
	Serial.print(input, HEX);
	Serial.println();
	Serial.print("input2:");
	Serial.print("\t 0x");
	Serial.print(input2, HEX);
	Serial.println();
#endif // DEBUG
}

void CH376::checkSuspended(uint8_t input, uint8_t input2) { exec20(CMD20_CHK_SUSPEND, input, input2); }
void CH376::setRetry(uint8_t input, uint8_t input2) { exec20(CMD20_SET_RETRY, input, input2); }
void CH376::setSDOINT(uint8_t input, uint8_t input2) { exec20(CMD20_SET_SDO_INT, input, input2); }
void CH376::writeVAR8(uint8_t input, uint8_t input2) { exec20(CMD20_WRITE_VAR8, input, input2); }
void CH376::writeOffsetData(uint8_t input, uint8_t input2) { exec20(CMD20_WR_OFS_DATA, input, input2, false); }
#pragma endregion

#pragma region CMDx0
void CH376::execx0(uint8_t CMDx0, uint8_t input[], int num, bool endTransfer) {
	spiBeginTransfer();
	spiWrite(CMDx0);
	for (int i = 0; i < num; i++) { spiWrite(input[i]); }
	if (endTransfer) { spiEndTransfer(); }
#ifdef DEBUG
	Serial.println();
	Serial.print("Command:");
	Serial.print("\t 0x");
	Serial.print(CMDx0, HEX);
	for (int i = 0; i < num; i++) {
		Serial.println();
		Serial.print("input");
		Serial.print(i);
		Serial.print(":");
		Serial.print("\t 0x");
		Serial.print(input[i], HEX);
	}
	Serial.println();
#endif // DEBUG
}

void CH376::setUSBID(uint8_t input, uint8_t input2, uint8_t input3, uint8_t input4) { 
	uint8_t inputs[4] = { input, input2, input3, input4 };
	execx0(CMD40_SET_USB_ID, inputs, 4);
}
void CH376::setFileSize(uint8_t input, uint8_t input2, uint8_t input3, uint8_t input4, uint8_t input5) {
	uint8_t inputs[5] = { input, input2, input3, input4, input5};
	execx0(CMD50_SET_FILE_SIZE, inputs, 5); 
}
void CH376::writeVAR32(uint8_t input, uint8_t input2, uint8_t input3, uint8_t input4, uint8_t input5) {
	uint8_t inputs[5] = { input, input2, input3, input4, input5};
	execx0(CMD50_WRITE_VAR32, inputs, 5); 
}
#pragma endregion

#pragma region CMD01
uint8_t CH376::exec01(uint8_t CMD01, bool endTransfer) {
	uint8_t tmpRet = 0;
	spiBeginTransfer();
	spiWrite(CMD01);
	tmpRet = spiRead();
	if (endTransfer) { spiEndTransfer(); }
#ifdef DEBUG
	Serial.println();
	Serial.print("Command:");
	Serial.print("\t 0x");
	Serial.print(CMD01, HEX);
	Serial.println();
	Serial.print("Returned:");
	Serial.print("\t 0x");
	Serial.print(tmpRet, HEX);
	Serial.println();
#endif // DEBUG
	return tmpRet;
}

uint8_t CH376::delay100US() { return exec01(CMD01_DELAY_100US); }
uint8_t CH376::getICVersion() { return exec01(CMD01_GET_IC_VER); }
uint8_t CH376::getInterrupt() { return exec01(CMD01_GET_STATUS); }
uint8_t CH376::readUSBData() { return exec01(CMD01_RD_USB_DATA, false); }
uint8_t CH376::readUSBData0() { return exec01(CMD01_RD_USB_DATA0, false); }
uint8_t CH376::testConnect() { return exec01(CMD01_TEST_CONNECT); }
uint8_t CH376::writeRequestedData() { return exec01(CMD01_WR_REQ_DATA, false); }
#pragma endregion

#pragma region CMD11
uint8_t CH376::exec11(uint8_t CMD11, uint8_t input, bool endTransfer) {
	uint8_t tmpRet = 0;
	spiBeginTransfer();
	spiWrite(CMD11);
	spiWrite(input);
	tmpRet = spiRead();
	if (endTransfer) { spiEndTransfer(); }
#ifdef DEBUG
	Serial.println();
	Serial.print("Command:");
	Serial.print("\t 0x");
	Serial.print(CMD11, HEX);
	Serial.println();
	Serial.print("Returned:");
	Serial.print("\t 0x");
	Serial.print(tmpRet, HEX);
	Serial.println();
#endif // DEBUG
	return tmpRet;
}

uint8_t CH376::CheckExist(uint8_t input) { return exec11(CMD11_CHECK_EXIST, input); }
USB_SPEED CH376::getDevRate(uint8_t input) { return (USB_SPEED)exec11(CMD11_GET_DEV_RATE, input); }
uint8_t CH376::getToggle(uint8_t input) { return exec11(CMD11_GET_TOGGLE, input); }
uint8_t CH376::readVar8(uint8_t input) { return exec11(CMD11_READ_VAR8, input); }
uint8_t CH376::setUSBMode(uint8_t input) {
	return exec11(CMD11_SET_USB_MODE, input);
	delayMicroseconds(40);
}
#pragma endregion

#pragma region CMD21
uint8_t CH376::exec21(uint8_t CMD21, uint8_t input, uint8_t input2, bool endTransfer) {
	uint8_t tmpRet = 0;
	spiBeginTransfer();
	spiWrite(CMD21);
	spiWrite(input);
	spiWrite(input2);
	tmpRet = spiRead();
	if (endTransfer) { spiEndTransfer(); }
#ifdef DEBUG
	Serial.println();
	Serial.print("Command:");
	Serial.print("\t 0x");
	Serial.print(CMD21, HEX);
	Serial.println();
	Serial.print("input:");
	Serial.print("\t 0x");
	Serial.print(input, HEX);
	Serial.println();
	Serial.print("input2:");
	Serial.print("\t 0x");
	Serial.print(input2, HEX);
	Serial.println();
	Serial.print("Returned:");
	Serial.print("\t 0x");
	Serial.print(tmpRet, HEX);
	Serial.println();
#endif // DEBUG
	return 	tmpRet;
}

uint8_t CH376::setBaudrate(uint8_t input, uint8_t input2) { return exec21(CMD21_SET_BAUDRATE, input, input2); }
#pragma endregion

#pragma region CMD0H
uint8_t CH376::exec0H(uint8_t CMD0H, bool endTransfer) {
	uint8_t tmpRet = 0;
	spiBeginTransfer();
	spiWrite(CMD0H);
	spiEndTransfer();
	tmpRet = waitInterrupt(endTransfer);
#ifdef DEBUG
	Serial.println();
	Serial.print("Command:");
	Serial.print("\t 0x");
	Serial.print(CMD0H, HEX);
	Serial.println();
	Serial.print("Returned:");
	Serial.print("\t 0x");
	Serial.print(tmpRet, HEX);
	Serial.println();
#endif // DEBUG
	return tmpRet;
}

uint8_t CH376::autoSetup() { return exec0H(CMD0H_AUTO_SETUP); }
uint8_t CH376::byteReadGo() {
	uint8_t tmpRet = exec0H(CMD0H_BYTE_RD_GO);

	if (tmpRet != USB_INT_DISK_READ && tmpRet != USB_INT_SUCCESS) {
		setError(tmpRet);
	}
	return tmpRet;
}
uint8_t CH376::byteWriteGo() {
	uint8_t tmpRet = exec0H(CMD0H_BYTE_WR_GO);

	if (tmpRet != USB_INT_DISK_WRITE && tmpRet != USB_INT_SUCCESS) {
		setError(tmpRet);
	}
	return tmpRet;
}
uint8_t CH376::dirCreate() { return exec0H(CMD0H_DIR_CREATE); }
uint8_t CH376::dirInfoSave() { return exec0H(CMD0H_DIR_INFO_SAVE); }
uint8_t CH376::diskBulgOnly(bool fillStruct) {
	uint8_t tmpRet = exec0H(CMD0H_DISK_BOC_CMD);

	if (tmpRet == USB_INT_SUCCESS) {
		if (fillStruct) {
			uint8_t tempDiskBocCbw[sizeof(BULK_ONLY_CBW)];
			uint8_t dataLength = readUSBData0();

			if (dataLength > sizeof(tempDiskBocCbw)) {
				setError(ERR_OVERFLOW);
			}
			else {
				spiReadMultiple(tempDiskBocCbw, dataLength);
				spiEndTransfer();
				memcpy(&DiskBocCbw, &tempDiskBocCbw, dataLength);
			}
		}
	}
	else { setError(tmpRet); }

	return tmpRet;
}
uint8_t CH376::diskCapacity() { return exec0H(CMD0H_DISK_CAPACITY); }
uint8_t CH376::diskConnect() { return exec0H(CMD0H_DISK_CONNECT); }
uint8_t CH376::diskInit(bool fillStruct) {
	uint8_t tmpRet = exec0H(CMD0H_DISK_INIT);

	if (tmpRet == USB_INT_SUCCESS) {
		if (fillStruct) {
			uint8_t tempDiskInitInq[sizeof (INQUIRY_DATA)];
			uint8_t dataLength = readUSBData0();

			if (dataLength > sizeof(tempDiskInitInq)) {
				setError(ERR_OVERFLOW);
			}
			else {
				spiReadMultiple(tempDiskInitInq, dataLength);
				spiEndTransfer();
				memcpy(&DiskInitInq, &tempDiskInitInq, dataLength);
			}
		}
	}
	else { setError(tmpRet); }

	return tmpRet;
}
uint8_t CH376::diskInquiry(bool fillStruct) {
	uint8_t tmpRet = exec0H(CMD0H_DISK_INQUIRY);

	if (tmpRet == USB_INT_SUCCESS) {
		if (fillStruct) {
			uint8_t tempDiskInqData[sizeof (INQUIRY_DATA)];
			uint8_t dataLength = readUSBData0();

			if (dataLength > sizeof(tempDiskInqData)) {
				setError(ERR_OVERFLOW);
			}
			else {
				spiReadMultiple(tempDiskInqData, dataLength);
				spiEndTransfer();
				memcpy(&DiskInqData, &tempDiskInqData, dataLength);
			}
		}
	}
	else { setError(tmpRet); }
	return tmpRet;
}
uint8_t CH376::diskMaxLogicalUnitNumber() { return exec0H(CMD0H_DISK_MAX_LUN); }
uint8_t CH376::diskMount(bool fillStruct) {
	uint8_t tmpRet = exec0H(CMD0H_DISK_MOUNT);

	if (tmpRet == USB_INT_SUCCESS) {
		if (fillStruct) {
			uint8_t tempDiskMountInq[sizeof(INQUIRY_DATA)];
			uint8_t dataLength = readUSBData0();

			if (dataLength > sizeof(tempDiskMountInq)) {
				setError(ERR_OVERFLOW);
			}
			else {
				spiReadMultiple(tempDiskMountInq, dataLength);
				spiEndTransfer();
				memcpy(&DiskMountInq, &tempDiskMountInq, dataLength);
			}
		}
	}
	else { setError(tmpRet); }
	return tmpRet;
}
uint8_t CH376::diskQuery(bool fillStruct) {
	uint8_t tmpRet = exec0H(CMD0H_DISK_QUERY);

	if (tmpRet == USB_INT_DISK_READ) {
		if (fillStruct) {
			uint8_t tmpDiskQueryInfo[sizeof(DiskQuery)];
			uint8_t dataLength = readUSBData0();

			if (dataLength > sizeof(tmpDiskQueryInfo)) {
				setError(ERR_OVERFLOW);
			}
			else {
				spiReadMultiple(tmpDiskQueryInfo, dataLength);
				spiEndTransfer();
				memcpy(&DiskQueryInfo, &tmpDiskQueryInfo, dataLength);
			}
		}
	}
	else { setError(tmpRet); }
	return tmpRet;
}
uint8_t CH376::diskReadGo() { return exec0H(CMD0H_DISK_RD_GO); }
uint8_t CH376::diskReady() { return exec0H(CMD0H_DISK_READY); }
uint8_t CH376::diskReset() { return exec0H(CMD0H_DISK_RESET); }
uint8_t CH376::diskCheckErrors(bool fillStruct) {
	uint8_t tmpRet = exec0H(CMD0H_DISK_R_SENSE);

	if (tmpRet == USB_INT_SUCCESS) {
		if (fillStruct) {
			uint8_t tempReqSenseData[sizeof (SENSE_DATA)];
			uint8_t dataLength = readUSBData0();

			if (dataLength > sizeof(tempReqSenseData)) {
				setError(ERR_OVERFLOW);
			}
			else {
				spiReadMultiple(tempReqSenseData, dataLength);
				spiEndTransfer();
				memcpy(&ReqSenseData, &tempReqSenseData, dataLength);
			}
		}
	}
	else { setError(tmpRet); }
	return tmpRet;
}
uint8_t CH376::diskSize() {
		return exec0H(CMD0H_DISK_SIZE);
}
uint8_t CH376::diskWriteGo() { return exec0H(CMD0H_DISK_WR_GO); }
uint8_t CH376::fileCreate() { return exec0H(CMD0H_FILE_CREATE); }
uint8_t CH376::fileEnumGo(bool fillStruct) {
	uint8_t tmpRet = exec0H(CMD0H_FILE_ENUM_GO);

	if (tmpRet == USB_INT_DISK_READ) {
		if (fillStruct) {
			uint8_t tempEnumDirInfo[sizeof(FAT_DIR_INFO)];
			uint8_t dataLength = readUSBData0();

			if (dataLength > sizeof(tempEnumDirInfo)) {
				setError(ERR_OVERFLOW);
			}
			else {
				spiReadMultiple(tempEnumDirInfo, dataLength);
				spiEndTransfer();
				memcpy(&EnumDirInfo, &tempEnumDirInfo, dataLength);
			}
		}
	}
	else { setError(tmpRet); }
	return tmpRet;
}
uint8_t CH376::fileErase() { return exec0H(CMD0H_FILE_ERASE); }
uint8_t CH376::fileOpen(bool fillStruct) {
	uint8_t tmpRet = exec0H(CMD0H_FILE_OPEN);

	if (tmpRet == USB_INT_SUCCESS) {
		if (fillStruct) {
			uint8_t tempOpenDirInfo[sizeof(FAT_DIR_INFO)];
			uint8_t dataLength = readUSBData0();

			if (dataLength > sizeof(tempOpenDirInfo)) {
				setError(ERR_OVERFLOW);
			}
			else {
				spiReadMultiple(tempOpenDirInfo, dataLength);
				spiEndTransfer();
				memcpy(&OpenDirInfo, &tempOpenDirInfo, dataLength);
			}
		}
	}
	else { setError(tmpRet); }
	return tmpRet;
}
uint8_t CH376::readDiskSector() { return exec0H(CMD0H_RD_DISK_SEC); }
uint8_t CH376::writeDiskSector() { return exec0H(CMD0H_WR_DISK_SEC); }
#pragma endregion

#pragma region CMD1H
uint8_t CH376::exec1H(uint8_t CMD1H, uint8_t input, bool endTransfer) {
	uint8_t tmpRet = 0;
	spiBeginTransfer();
	spiWrite(CMD1H);
	spiWrite(input);
	spiEndTransfer();
	tmpRet = waitInterrupt(endTransfer);
#ifdef DEBUG
	Serial.println();
	Serial.print("Command:");
	Serial.print("\t 0x");
	Serial.print(CMD1H, HEX);
	Serial.println();
	Serial.print("Returned:");
	Serial.print("\t 0x");
	Serial.print(tmpRet, HEX);
	Serial.println();
#endif // DEBUG
	return tmpRet;
}

uint8_t CH376::clearEndPointError(uint8_t input) { return exec1H(CMD1H_CLR_STALL, input); }
uint8_t CH376::dirInfoRead(uint8_t input) { return exec1H(CMD1H_DIR_INFO_READ, input); }
uint8_t CH376::fileClose(uint8_t input) { return exec1H(CMD1H_FILE_CLOSE, input); }
uint8_t CH376::getDescription(uint8_t input) { return exec1H(CMD1H_GET_DESCR, input); }
uint8_t CH376::issueToken(uint8_t input) { return exec1H(CMD1H_ISSUE_TOKEN, input); }
uint8_t CH376::readFromSector(uint8_t input) { return exec1H(CMD1H_SEC_READ, input); }
uint8_t CH376::writeToSector(uint8_t input) { return exec1H(CMD1H_SEC_WRITE, input); }
uint8_t CH376::setAddress(uint8_t input) { return exec1H(CMD1H_SET_ADDRESS, input); }
uint8_t CH376::setConfig(uint8_t input) { return exec1H(CMD1H_SET_CONFIG, input); }
#pragma endregion

#pragma region CMD2H
uint8_t CH376::exec2H(uint8_t CMD2H, uint8_t input, uint8_t input2, bool endTransfer) {
	uint8_t tmpRet = 0;
	spiBeginTransfer();
	spiWrite(CMD2H);
	spiWrite(input);
	spiWrite(input2);
	spiEndTransfer();
	tmpRet = waitInterrupt(endTransfer);
#ifdef DEBUG
	Serial.println();
	Serial.print("Command:");
	Serial.print("\t 0x");
	Serial.print(CMD2H, HEX);
	Serial.println();
	Serial.print("input:");
	Serial.print("\t 0x");
	Serial.print(input, HEX);
	Serial.println();
	Serial.print("input2:");
	Serial.print("\t 0x");
	Serial.print(input2, HEX);
	Serial.println();
	Serial.print("Returned:");
	Serial.print("\t 0x");
	Serial.print(tmpRet, HEX);
	Serial.println();
#endif // DEBUG
	return 	tmpRet;
}

uint8_t CH376::readByte(uint8_t input, uint8_t input2) {
	uint8_t tmpRet = exec2H(CMD2H_BYTE_READ, input, input2);

	if (tmpRet != USB_INT_SUCCESS && tmpRet != USB_INT_DISK_READ) {
		setError(tmpRet);
	}
	return tmpRet;
}
uint8_t CH376::writeByte(uint8_t input, uint8_t input2) {
	uint8_t tmpRet = exec2H(CMD2H_BYTE_WRITE, input, input2);

	if (tmpRet != USB_INT_SUCCESS && tmpRet != USB_INT_DISK_WRITE) {
		setError(tmpRet);
	}
	return tmpRet;
}
uint8_t CH376::issueToken(uint8_t input, uint8_t input2) { return exec2H(CMD2H_ISSUE_TKN_X, input, input2); }
#pragma endregion

#pragma region CMDxH
uint8_t CH376::execxH(uint8_t CMDxH, uint8_t input[], int num, bool endTransfer) {
	uint8_t tmpRet = 0;
	spiBeginTransfer();
	spiWrite(CMDxH);
	for (int i = 0; i < num; i++) { spiWrite(input[i]); }
	spiEndTransfer();
	tmpRet = waitInterrupt(endTransfer);
#ifdef DEBUG
	Serial.println();
	Serial.print("Command:");
	Serial.print("\t 0x");
	Serial.print(CMDxH, HEX);
	for (int i = 0; i < num; i++) {
		Serial.println();
		Serial.print("input");
		Serial.print(i);
		Serial.print(":");
		Serial.print("\t 0x");
		Serial.print(input[i], HEX);
	}
	Serial.println();
	Serial.print("Returned:");
	Serial.print("\t 0x");
	Serial.print(tmpRet, HEX);
	Serial.println();
#endif // DEBUG
	return 	tmpRet;
}

uint8_t CH376::movePointer(uint8_t input, uint8_t input2, uint8_t input3, uint8_t input4) {
	uint8_t inputs[4] = { input, input2, input3, input4 };
	return execxH(CMD4H_BYTE_LOCATE, inputs, 4);
}
uint8_t CH376::moveSectorPointer(uint8_t input, uint8_t input2, uint8_t input3, uint8_t input4) {
	uint8_t inputs[4] = { input, input2, input3, input4 };
	return execxH(CMD4H_SEC_LOCATE, inputs, 4); 
}
uint8_t CH376::diskRead(uint8_t input, uint8_t input2, uint8_t input3, uint8_t input4, uint8_t input5) {
	uint8_t inputs[4] = { input, input2, input3, input4 };
	return execxH(CMD5H_DISK_READ, inputs, 5); 
}
uint8_t CH376::diskWrite(uint8_t input, uint8_t input2, uint8_t input3, uint8_t input4, uint8_t input5) {
	uint8_t inputs[4] = { input, input2, input3, input4 };
	return execxH(CMD5H_DISK_WRITE, inputs, 5); 
}
#pragma endregion
