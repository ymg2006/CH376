# Arduino library for CH376 usb host and file manager control chip

Supports read/write files to USB flash drive or SD card (CH375 only support USB flash drive).

> Why use this chip if there is already a library to handle the SD card and it is easier to just hook up the SD card(with resistors or SD card module) to Arduino?
> The SD library is widely used and is reliable, the only problem is the Arduino does't have to much memory and with the SD lib the MCU has to cope with the FAT file system,
> and we're just talking about SD card management, the USB storage drive handling is a more complicated and memory consuming procedure and you need a USB-HOST chip.
> The CH376 chip easily can write and read files even if they are on SD card or on Usb thumb-drive(CH375 only support USB thumb-drive). The chip supports FAT12, FAT16 and FAT32 file systems, meaning the chip does the hard work, 
> the MCU does not have to deal with the FAT file system, it only sends instructions to the chip on the communication bus you like (SPI, UART (HW serial, SW serial)), and the magic happens in the chip.
> The chip can do more, e.g to handle HID devices(usb keyboard, mouse, joystick ...) but this feature is not yet used in the library, planned to be in the future.

## Known Issues

<span style="background-color:#dcca5c">Some of these chips are shipped with old firmware( revision number less than 0x43), with these modules you will faces some issues like doesn't recognize a flash drive or it will be unreadable the file created with this chip for Windows and vice versa. For more details [click here.](https://github.com/ymg2006/CH376#firmware-difference)</span>

## Getting Started

Configure the jumpers on the module depending on which communication protocol you are using(see [API reference](https://github.com/ymg2006/CH376#api-reference))
![Alt text](extras/JumperSelect.png?raw=true "Setting")

## API Reference

```C++
//The default SPI communication speed is reduced to 125 kHz because of stability if long cables or breadboard is used. 
// to change the SPI Clock rate, during instantiation use e.g. SPI_SCK_KHZ(500) - to use 500kHz
//                                                     or e.g. SPI_SCK_MHZ(8) - to use 8MHz (see in examples/lcd_menu)
    //CONSTRUCTORS
       //UART
     //For hardware serial leave the communication settings on the module at default speed (9600bps) 
    Ch376msc(HardwareSerial, speed);//Select the serial port to which the module is connected and the desired speed(9600, 19200, 57600, 115200)

     //For software serial select the desired communication speed on the module(look on the picture above)
    Ch376msc(SoftwareSerial);

       //SPI
     //If no other device is connected to the SPI port it`s possible to save one MCU pin
    Ch376msc(spiSelect, *optional SPI CLK rate*);// ! Don`t use this if the SPI port is shared with other devices

     //If the SPI port is shared with other devices, use this constructor and one extra MCU pin need to be sacrificed for the INT pin
    Ch376msc(spiSelect, interruptPin, *optional SPI CLK rate*);
    ////////////////////

     // Must be initialized before any other command are called from this class.
    init();

     // call frequently to get any interrupt message of the module(attach/detach drive)
    checkIntMessage(); //return TRUE if an interrupt request has been received, FALSE if not.

     // can call before any file operation
    driveReady(); //returns FALSE if no drive is present or TRUE if drive is attached and ready.

     // check the communication between MCU and the CH376
    pingDevice(); //returns FALSE if there is a communication failure, TRUE if communication  is ok

     // 8.3 filename, also called a short filename is accepted 
    setFileName(filename);//8 char long name + 3 char long extension

     // open file before any file operation. Use first setFileName() function
    openFile();

     // always call this after finishing with file operations otherwise data loss or file corruption may occur
    closeFile();

     // repeatedly call this function to read data to buffer until the return value is TRUE
    readFile(buffer, length);// buffer - char array, buffer size

     // Read text until reach the terminator character, rest is same as readFile
    readFileUntil(terminator, buffer, length);//returns boolean true if the given buffer
                                        //      is full and not reached the terminator character

     //Same as readFile except the buffer type is byte(uint8) array and not added terminating 0 char
    readRaw(buffer, length);// buffer - byte array, buffer size

     //Read, extract numbers of txt file, read until reach EOF (see getEOF())
    readLong(terminator);//returns long value,terminator char is optional, default char is '\n'
    readULong(terminator);//returns unsigned long value,terminator char is optional, default char is '\n'
    readDouble(terminator);//returns double value,terminator char is optional, default char is '\n'

     //Write, construct string of number and write on the storage(byte, int, u int, long, u long, double)
    writeNum(number);// write the given number
    writeNumln(number);// write the given number in new line

     //Write one character on the storage
    writeChar(char);// e.g. new line character '\n' or comma ',' to 

     // repeatedly call this function to write data to the drive until there is no more data for write or the return value is FALSE
    writeFile(buffer, length);// buffer - char array, string size in the buffer

     // switch between source drive's, 0 = USB(default), 1 = SD card
     // !!Before calling this function and activate the SD card please do the required modification 
     // on the pcb, please read **PCB modding for SD card** section otherwise you can damage the CH376 chip.
    setSource(srcDrive);// 0 or 1

    setYear(year); // 1980 - 2099
    setMonth(month);// 1 - 12
    setDay(day);// 1 - 31
    setHour(hour);// 0 - 23
    setMinute(minute);// 0 - 59
    setSecond(second);// 0 - 59 saved with 2 second resolution (0, 2, 4 ... 58)

     // when new file is created the defult file creation date/time is (2004-1-1 0.0.0), 
     // it is possible to change date/time with this function, use first set functions above to set the file attributes
    saveFileAttrb();

     // move the file cursor to specified position
    moveCursor(position);// 00000000h - FFFFFFFFh

     // delete the specified file, use first setFileName() function
    deleteFile();

    // delete current directory, except root directory
    deleteDir();

     // repeatedly call this function with getFileName until the return value is TRUE to get the file names from the current directory
     // limited possibility to use with wildcard character e.g. listDir("AB*") will list files with names starting with AB
     // listDir("*AB") will not work, wildcard char+string must to be less than 8 character long
     // if no argument is passed while calling listDir(), all files will be printed from the current directory
    listDir();// returns FALSE if no more file is in the current directory

     // reset file process state machine to default
     // useful e.g. to make LCD menu with file's list without using large buffer to store the file names
    resetFileList();

     //dirPath = e.g. "/DIR1/DIR2/DIR3" , "/" - root dir
     //CreateDir = 0(open directories if they not exist, don`t create them) or 1(create directories if they do not exist and open them)
     //if working in subfolders, before file operations ALWAYS call this function with the full directory path
     //limited to 3 subfolders depth (see /src/Ch376msc.h file. MAXDIRDEPTH) and 8 character long directory names
    cd(dirPath,CreateDir);// returns byte value,see example .ino

    getFreeSectors();// returns unsigned long value
    getTotalSectors();// returns unsigned long value
    getFileSize();// returns unsigned long value (byte)
    getSource();// returns boolean value, false USB, true SD card

    getYear();// returns int value
    getMonth();// returns int value
    getDay();// returns int value
    getHour();// returns int value
    getMinute();// returns int value
    getSecond();// returns int value

     // get the last error code (see datasheet and/or CommDef.h)
    getError();// returns byte value

    getFileSystem();// returns byte value, 01h-FAT12, 02h-FAT16, 03h-FAT32
    getFileName();// returns the file name in a 11+1 character long string value
    getFileSizeStr();// returns file size in a formatted 9+1 character long string value
    getFileAttrb();// returns byte value, see /src/CommDef.h , (File attributes)
    getCursorPos();// returns unsigned long value
    getEOF();// returns boolean value, true EOF is reached
    getChipVer();// returns byte value, returns the CH chip firmware version number
    getStreamLen();//returns byte value, helper function to readRaw() function, get the stream size issue#35
```

## Firmware difference

I finally managed to get a module with outdated firmware and a PC with Win10 OS, so I was able to experiment with what some are complaining about. Modules with an obsolete firmware and a flash drive formatted with Windows 10 inbuilt tool (right click on the drive and choose Format option) are a bad combination (didn't tested with earlier version of Windows OS ). In this case You can try to install a third-party software and do the formatting with that tool, for me on Windows the [Active@ Partition Manager](https://www.lsoft.net/partition-manager/index.html) from LSoft Technologies did the job.

This test is based on my personal experience and may vary depending on the flash drives you use.

With calling getChipVer() function you can determine your chip's firmware revision number. 

**Pendrives factory default formatted or formatted with Windows 10**

| FW ver.        | 2Gb    | 16Gb  | 32Gb   |
| -------------- |:------:|:-----:|:------:|
| 0x43 (67)(new) | OK     | OK    | OK     |
| 0x42 (66)(old) | **FAIL | *FAIL | **FAIL |

**Pendrives are formatted with GParted on Linux or with third-party software on Windows**

| FW ver.        | 2Gb | 16Gb  | 32Gb |
| -------------- |:---:|:-----:|:----:|
| 0x43 (67)(new) | OK  | OK    | OK   |
| 0x42 (66)(old) | OK  | *FAIL | OK   |

> *Can't mount the drive
> 
> **Can't read the drive or file created with the module is unreadable for Windows and vice versa

Flash drives used in test:

> NoName 2Gb USB2.0, IdVendor: 1234, IdProduct: 0201 Brain Actuated Technologies
> 
> PNY 16Gb USB2.0, IdVendor: 058f, IdProduct: 6387 Alcor Micro Corp.
> 
> PNY 32Gb USB3.1, IdVendor: 090c, IdProduct: 2000 Silicon Motion, Inc.

## Tested boards

| Board(arch)                                                  | SPI              | HW Serial | SW Serial |
|:------------------------------------------------------------ |:----------------:|:---------:|:---------:|
| Arduino (AVR)                                                | OK               | OK        | OK        |
| DUE (SAM)                                                    | OK(with INT pin) | OK        | NO        |
| ZERO (SAMD)                                                  | OK               | ?         | NO        |
| *STM32 cores                                                 | OK               | !NO       | NO        |
| **STM32duino                                                 | OK               | OK        | NO        |
| ***ESP8266                                                   | OK(with INT pin) | NO        | OK        |
| ESP32                                                        | OK               | OK        | OK        |

Be careful when choosing SoftSerial because it has its own limitations.

> `*` Tested on NUCLEO F446RE(no signal at all on UART ports)

> `**` Tested on Generic STM32F103C alias Blue pill with STM32duino bootloader

> `***` Tested on NodeMCU,(i'm not familiar with ESP MCUs) it looks they have default enabled WDT so i have to call
>    `yield()` periodically during file operations, otherwise ESP will restart with a ugly message.
>    Working SPI configuration (for me)is MISO-12(D6), MOSI-13(D7), SCK-14(D5), CS-4(D2), INT-5(D1)

## Acknowledgments

Thanks for the file manager library [György Kovács](https://github.com/djuseeq/Ch376msc)

## Versions

v1.0 Jan 24, 2022

- initial version with SPI communication

## License

The MIT License (MIT)

Copyright (c) 2022 Farid Partonia

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
