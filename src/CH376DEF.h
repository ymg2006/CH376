/* C Define for CH376
/* Website:  http://wch.cn   */
/* Email:    tech@wch.cn     */
/* Author:   W.ch 2008.10    */
/* V1.0 for CH376            */

#ifndef __CH376INC_H__
#define __CH376INC_H__
#ifdef __cplusplus
extern "C" {
#endif
	/* ********************************************************************************************************************* */
	/* common types and constant definitions */
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif
#ifndef UINT8
	typedef unsigned char UINT8;
#endif
#ifndef UINT16
	typedef unsigned short UINT16;
#endif
#ifndef UINT32
	typedef unsigned long UINT32;
#endif
#ifndef PUINT8
	typedef unsigned char* PUINT8;
#endif
#ifndef PUINT16
	typedef unsigned short* PUINT16;
#endif
#ifndef PUINT32
	typedef unsigned long* PUINT32;
#endif
#ifndef UINT8V
	typedef unsigned char volatile UINT8V;
#endif
#ifndef PUINT8V
	typedef unsigned char volatile* PUINT8V;
#endif

#define ANSWTIMEOUT 1000

#define ERR_OVERFLOW 0x03
#define ERR_TIMEOUT 0x02
#define ERR_NO_RESPONSE 0x01
#define ERR_LONGFILENAME 0x04

	/* ********************************************************************************************************************* */
	/* hardware features */
#define CH376_DAT_BLOCK_LEN 0x40 /* USB single packet, maximum length of data block, length of default buffer */
/* ********************************************************************************************************************* */
/* command code */
/* SOME COMMANDS ARE COMPATIBLE WITH CH375 CHIPS, BUT THE INPUT DATA OR OUTPUT DATA MAY BE LOCALLY DIFFERENT) */
/* a command operation sequence contains:
one command code (for serial port mode, the command code needs two synchronization codes before it),
several input data (can be 0),
generate an interrupt notification or several output data (which can be 0), choose one of the two, if there is an interrupt notification, there must be no output data, and if there is output data, there must be no interruption
WITH THE EXCEPTION OF CMD01_WR_REQ_DATA COMMANDS, THE ORDER CONSISTS OF: ONE COMMAND CODE, ONE OUTPUT DATA, AND SEVERAL INPUT DATA
Command code naming rules: CMDxy_NAME
where x and y are both numbers, x indicates the minimum number of input data (bytes), y indicates the minimum number of output data (bytes), y if it is H, it means that an interrupt notification is generated.
some commands are capable of reading and writing to blocks from 0 to multiple bytes, and the number of bytes in the block itself is not included in the x or y above */
/* By default, this file will also provide a command code format compatible with the CH375 chip command code (i.e. after removing x and y), if not, then you can define _NO_CH375_COMPATIBLE_ disable */
/* ********************************************************************************************************************* */
/* main commands (manual 1), common */
#define CMD01_GET_IC_VER 0x01 /* Obtain chip and firmware version */
/* output: version number (bit 7 is 0, bit 6 is 1, bit 5~bit 0 is version number ) */
/* CH376 RETURNS A VERSION NUMBER OF 041H, I.E. A VERSION NUMBER OF 01H */
#define CMD21_SET_BAUDRATE 0x02 /* Serial port mode: Set the port communication baud rate (the default baud rate after power-up or reset is 9600bps, selected by the D4/D5/D6 pins) */
/* input: baud rate divider coefficient, baud rate divide constant */
/* OUTPUT: OPERATION STATUS ( CMD_RET_SUCCESS OR CMD_RET_ABORT, OTHER VALUES INDICATE THAT THE OPERATION IS NOT COMPLETED ) */
#define CMD00_ENTER_SLEEP 0x03 /* Go to sleep */
#define CMD00_RESET_ALL 0x05 /* Perform a hardware reset */
#define CMD11_CHECK_EXIST 0x06 /* Test communication interface and operating status */
/* enter: arbitrary data */
/* output: bitwise reverse of input data */
#define CMD20_CHK_SUSPEND 0x0B /* Device Mode: Set the way to check the USB bus hang status */
/* INPUT: DATA 10H, CHECK METHOD */
/* 00H=Do not check for USB hangs, 04H=Check for USB hangs at 50mS intervals, 05H=Check for USB hangs at 10mS intervals */
#define CMD20_SET_SDO_INT 0x0B /* SPI Interface Mode: Set the interrupt mode of the SDO pin of the SPI */
/* INPUT: DATA 16H, INTERRUPT MODE */
/* 10H=DISABLE SDO PIN FOR INTERRUPT OUTPUT, THREE-STATE OUTPUT DISABLE WHEN SCS CHIP SELECTION IS INVALID, 90H=SDO PIN DOUBLE INTERRUPT REQUEST OUTPUT WHEN SCS CHIP SELECTION IS INVALID */
#define CMD14_GET_FILE_SIZE 0x0C /* Hosts File Mode: Gets the current file length */
/* INPUT: DATA 68H */
/* output: current file length (total length 32 bits, low bytes first) */
#define CMD50_SET_FILE_SIZE 0x0D /* Hosts File Mode: Sets the current file length */
/* INPUT: DATA 68H, CURRENT FILE LENGTH (TOTAL LENGTH 32 BITS, LOW BYTES FIRST) */
#define CMD11_SET_USB_MODE 0x15 /* Set USB working mode */
/* enter: mode code */
/* 00H=DEVICE MODE NOT ENABLED, 01H=ENABLED DEVICE MODE AND USING EXTERNAL FIRMWARE MODE (SERIAL PORT NOT SUPPORTED), 02H=ENABLED DEVICE MODE AND USING BUILT-IN FIRMWARE MODE */
/* 03H= SD CARD HOST MODE / UNENLIGHTENED HOST MODE FOR MANAGING AND ACCESSING FILES IN SD CARD */
/* 04H=HOST MODE NOT ENABLED, 05H=ENABLED HOST MODE, 06H=ENABLED HOST MODE AND AUTOMATICALLY GENERATES SOF PACKETS, 07H=ENABLED HOST MODE AND RESET USB BUS */
/* OUTPUT: OPERATION STATUS ( CMD_RET_SUCCESS OR CMD_RET_ABORT, OTHER VALUES INDICATE THAT THE OPERATION IS NOT COMPLETED ) */
#define CMD01_GET_STATUS 0x22 /* Get interrupt status and cancel interrupt requests */
/* output: interrupt status */
#define CMD00_UNLOCK_USB 0x23 /* Device mode: Release the current USB buffer */
#define CMD01_RD_USB_DATA0 0x27 /* Read data blocks from the endpoint buffer of the current USB interrupt or the receive buffer of the host endpoint */
/* output: length, data stream */
#define CMD01_RD_USB_DATA 0x28 /* Device mode: Reads a block of data from the endpoint buffer of the current USB interrupt and frees the buffer, equivalent to CMD01_RD_USB_DATA0 + CMD00_UNLOCK_USB */
/* output: length, data stream */
#define CMD10_WR_USB_DATA7 0x2B /* Device mode: Write data blocks to the transmit buffer of USB endpoint 2 */
/* input: length, data stream */
#define CMD10_WR_HOST_DATA 0x2C /* Write data blocks to the send buffer of the USB host endpoint */
/* input: length, data stream */
#define CMD01_WR_REQ_DATA 0x2D /* Write the requested block to the internally specified buffer */
/* output: length */
/* input: data stream */
#define CMD20_WR_OFS_DATA 0x2E /* Write a block of data to the internal buffer by specifying an offset address */
/* input: offset, length, stream */
#define CMD10_SET_FILE_NAME 0x2F /* Hosts File Mode: Set the file name of the file that will be manipulated */
/* enter: string ending in 0 (no more than 14 characters in length with terminator 0) */
/* ********************************************************************************************************************* */
/* main command (manual 1), common, the following command always produces an interrupt notification at the end of the operation, and always has no output data */
#define CMD0H_DISK_CONNECT 0x30 /* Host File Mode/SD Card Not Supported: Check if the disk is connected */
/* output interrupt */
#define CMD0H_DISK_MOUNT 0x31 /* Hosts File Mode: Initialize the disk and test that the disk is ready */
/* output interrupt */
#define CMD0H_FILE_OPEN 0x32 /* Host File Mode: Open files or directories (folders), or enumerate files and directories (folders) */
/* output interrupt */
#define CMD0H_FILE_ENUM_GO 0x33 /* Hosts File Mode: Continue enumerating files and directories (folders) */
/* output interrupt */
#define CMD0H_FILE_CREATE 0x34 /* Hosts File Mode: Create a new file, delete it if it already exists */
/* output interrupt */
#define CMD0H_FILE_ERASE 0x35 /* Hosts File Mode: Deletes files, deletes them directly if they are already open, otherwise they will be opened and then deleted for files, and subdirectories must be opened first */
/* output interrupt */
#define CMD1H_FILE_CLOSE 0x36 /* Hosts File Mode: Closes the currently open file or directory (folder) */
/* input: whether to allow update file length */
/* 00H=FORBIDDEN UPDATE LENGTH, 01H=ALLOWED UPDATE LENGTH */
/* output interrupt */
#define CMD1H_DIR_INFO_READ 0x37 /* Hosts File Mode: Read directory information for files */
/* input: specify the index number within the sector of the directory information structure that needs to be read */
/* INDEX NUMBER RANGES FROM 00H TO 0FH, INDEX NUMBER 0FFH IS THE FILE THAT IS CURRENTLY OPEN */
/* output interrupt */
#define CMD0H_DIR_INFO_SAVE 0x38 /* Hosts File Mode: Saves file directory information */
/* output interrupt */
#define CMD4H_BYTE_LOCATE 0x39 /* Host File Mode: Moves the current file pointer in bytes */
/* input: number of offset bytes (total length 32 bits, low bytes first) */
/* output interrupt */
#define CMD2H_BYTE_READ 0x3A /* Hosts File Mode: Reads blocks of data from the current location in bytes */
/* input: the number of bytes requested to be read (total length 16 bits, low bytes first) */
/* output interrupt */
#define CMD0H_BYTE_RD_GO 0x3B /* Hosts File Mode: Continue byte Read */
/* output interrupt */
#define CMD2H_BYTE_WRITE 0x3C /* Hosts file mode: Writes blocks of data to the current location in bytes */
/* input: the number of bytes requested to be written (total length 16 bits, low bytes first) */
/* output interrupt */
#define CMD0H_BYTE_WR_GO 0x3D /* Host File Mode: Continue Byte Write */
/* output interrupt */
#define CMD0H_DISK_CAPACITY 0x3E /* Hosts File Mode: Query disk physical capacity */
/* output interrupt */
#define CMD0H_DISK_QUERY 0x3F /* Hosts File Mode: Query disk space information */
/* output interrupt */
#define CMD0H_DIR_CREATE 0x40 /* Host File Mode: Create a new directory (folder) and open it, open it directly if the directory already exists */
/* output interrupt */
#define CMD4H_SEC_LOCATE 0x4A /* Host File Mode: Moves the current file pointer in sectors */
/* input: number of offset sectors (total length 32 bits, low bytes first) */
/* output interrupt */
#define CMD1H_SEC_READ 0x4B /* Host file mode / SD card not supported: read data blocks from the current position in sectors */
/* enter: number of sectors requested to be read */
/* output interrupt */
#define CMD1H_SEC_WRITE 0x4C /* Host file mode/ SD card not supported: Write data blocks in sectors at the current location */
/* enter: number of sectors requested to write */
/* output interrupt */
#define CMD0H_DISK_BOC_CMD 0x50 /* Host mode/SD card not supported: Command to execute The BulgOnly transfer protocol on USB memory */
/* output interrupt */
#define CMD5H_DISK_READ 0x54 /* Host mode/SD card not supported: Read physical sectors from USB memory */
/* INPUT: LBA PHYSICAL SECTOR ADDRESS (TOTAL LENGTH 32 BITS, LOW BYTES IN FRONT), NUMBER OF SECTORS (01H~FFH) */
/* output interrupt */
#define CMD0H_DISK_RD_GO 0x55 /* Host mode/SD card not supported: Continue with physical sector read operations of USB memory */
/* output interrupt */
#define CMD5H_DISK_WRITE 0x56 /* Host mode/SD card not supported: Write physical sectors to USB memory */
/* INPUT: LBA PHYSICAL SECTOR ADDRESS (TOTAL LENGTH 32 BITS, LOW BYTES IN FRONT), NUMBER OF SECTORS (01H~FFH) */
/* output interrupt */
#define CMD0H_DISK_WR_GO 0x57 /* Host mode/SD card not supported: Continue writing physical sectors of USB memory */
/* output interrupt */
/* ********************************************************************************************************************* */
/* AUXILIARY COMMANDS (MANUAL II), LESS COMMONLY USED OR FOR COMPATIBILITY WITH CH375 AND CH372 */
#define CMD10_SET_USB_SPEED 0x04 /* Set THE USB bus speed, which automatically reverts to 12Mbps full speed every time CMD11_SET_USB_MODE set the USB working mode */
/* input: bus speed code */
/* 00H=12Mbps FullSpeed (default), 01H=1.5Mbps (frequency modified only), 02H=1.5Mbps LowSpeed */
#define CMD11_GET_DEV_RATE 0x0A /* Host Mode: Gets the data rate type of the currently connected USB device */
/* INPUT: DATA 07H */
/* output: data rate type */
/* Bit 4 for 1 is a 1.5Mbps low-speed USB device, otherwise it is a 12Mbps full-speed USB device */
#define CMD11_GET_TOGGLE 0x0A /* Get the synchronization status of an OUT transaction */
/* INPUT: DATA 1AH */
/* output: sync status */
/* BIT 4 IS 1 THEN THE OUT TRANSACTION IS SYNCHRONIZED, OTHERWISE THE OUT TRANSACTION IS NOT SYNCHRONIZED */
#define CMD11_READ_VAR8 0x0A /* Read the specified 8-bit file system variable */
/* enter: variable address */
/* output: data */
/*#define CMD11_GET_MAX_LUN = CMD11_READ_VAR8( VAR_UDISK_LUN )*/ /* Host Mode: Get USB memory maximum and current logical unit number */
#define CMD20_SET_RETRY 0x0B /* Host Mode: Set the number of retries for USB transaction operations */
/* INPUT: DATA 25H, NUMBER OF RETRIES */
/* BIT 7 IS 0 AND DOES NOT RETRY WHEN A NAK IS RECEIVED, BIT 7 IS 1 BIT 6 IS 0 AND RECEIVES UNLIMITED RETRIES WHEN A NAK IS RECEIVED, BIT 7 IS 1 BIT 6 IS 1 AND CAN BE RETRYED FOR UP TO 3 SECONDS WHEN RECEIVING A NAK, BITS 5 TO 0 IS THE NUMBER OF RETRIES AFTER TIMEOUT */
#define CMD20_WRITE_VAR8 0x0B /* Set the specified 8-bit file system variable */
/* input: variable address, data */
/*#define CMD20_SET_DISK_LUN = CMD20_WRITE_VAR8( VAR_UDISK_LUN )*/ /* Host mode: Sets the current logical unit number of the USB memory */
#define CMD14_READ_VAR32 0x0C /* Read the specified 32-bit file system variable */
/* enter: variable address */
/* output: data (total length 32 bits, low bytes before) */
#define CMD50_WRITE_VAR32 0x0D /* Set the specified 32-bit file system variable */
/* input: variable address, data (total length 32 bits, low bytes first) */
#define CMD01_DELAY_100US 0x0F /* Delay 100uS (serial port not supported) */
/* output: output 0 during delay, end of delay output non-0 */
#define CMD40_SET_USB_ID 0x12 /* Device Mode: Set up USB vendor VID and product PID */
/* INPUT: MANUFACTURER ID LOW BYTE, MAKER ID HIGH BYTE, PRODUCT ID LOW BYTE, PRODUCT ID HIGH BYTE */
#define CMD10_SET_USB_ADDR 0x13 /* Set USB address */
/* enter: address value */
#define CMD01_TEST_CONNECT 0x16 /* Host mode/SD card not supported: Check usb device connection status */
/* OUTPUT: STATUS ( USB_INT_CONNECT OR USB_INT_DISCONNECT OR USB_INT_USB_READY, OTHER VALUES INDICATE THAT THE OPERATION IS NOT COMPLETED ) */
#define CMD00_ABORT_NAK 0x17 /* Host Mode: Discards retry of the current NAK */
#define CMD10_SET_ENDP2 0x18 /* Device mode (serial port not supported): Set the receiver for USB endpoint 0 */
/* input: how it works */
/* bit 7 is 1 then bit 6 is the synchronous trigger bit, otherwise the synchronous trigger bit is unchanged */
/* BITS 3~BIT 0 FOR TRANSACTION RESPONSE MODE: 0000-READY ACK, 1110-BUSY NAK, 1111-ERROR CALL*/
#define CMD10_SET_ENDP3 0x19 /* Device mode (serial port not supported): Set transmitter for USB endpoint 0 */
/* input: how it works */
/* bit 7 is 1 then bit 6 is the synchronous trigger bit, otherwise the synchronous trigger bit is unchanged */
/* BITS 3~BIT 0 ARE TRANSACTION RESPONSE MODES: 0000~1000-READY ACK, 1110-BUSY NAK, 1111-ERROR CALL */
#define CMD10_SET_ENDP4 0x1A /* Device mode (serial port not supported): Set the receiver of USB endpoint 1 */
/* input: how it works */
/* bit 7 is 1 then bit 6 is the synchronous trigger bit, otherwise the synchronous trigger bit is unchanged */
/* BITS 3~BIT 0 FOR TRANSACTION RESPONSE MODE: 0000-READY ACK, 1110-BUSY NAK, 1111-ERROR CALL*/
#define CMD10_SET_ENDP5 0x1B /* Device mode (serial port not supported): Set up transmitter for USB endpoint 1 */
/* input: how it works */
/* bit 7 is 1 then bit 6 is the synchronous trigger bit, otherwise the synchronous trigger bit is unchanged */
/* BITS 3~BIT 0 ARE TRANSACTION RESPONSE MODES: 0000~1000-READY ACK, 1110-BUSY NAK, 1111-ERROR CALL */
#define CMD10_SET_ENDP6 0x1C /* Set up the receiver of the USB endpoint 2/Host endpoint */
/* input: how it works */
/* bit 7 is 1 then bit 6 is the synchronous trigger bit, otherwise the synchronous trigger bit is unchanged */
/* BITS 3~BIT 0 ARE TRANSACTION RESPONSE MODES: 0000-READY ACK, 1101-READY BUT NOT RETURNING ACK, 1110-BUSY NAK, 1111-ERROR CALL */
#define CMD10_SET_ENDP7 0x1D /* Set up a transmitter for USB endpoint 2/host endpoint */
/* input: how it works */
/* bit 7 is 1 then bit 6 is the synchronous trigger bit, otherwise the synchronous trigger bit is unchanged */
/* BITS 3~BIT 0 ARE TRANSACTION RESPONSE MODES: 0000-READY ACK, 1101-READY BUT NO ANSWER, 1110-BUSY NAK, 1111-ERROR CALL*/
#define CMD00_DIRTY_BUFFER 0x25 /* Host File Mode: Clear internal disk and file buffers */
#define CMD10_WR_USB_DATA3 0x29 /* Device mode (serial port not supported): Write data blocks to the transmit buffer of USB endpoint 0 */
/* input: length, data stream */
#define CMD10_WR_USB_DATA5 0x2A /* Device mode (serial port not supported): Write data blocks to the transmit buffer of USB endpoint 1 */
/* input: length, data stream */
/* ********************************************************************************************************************* */
/* AUXILIARY COMMANDS (MANUAL II), LESS COMMONLY USED OR FOR COMPATIBILITY WITH CH375 AND CH372, THE FOLLOWING COMMANDS ALWAYS GENERATE AN INTERRUPT NOTIFICATION AT THE END OF THE OPERATION AND ALWAYS HAVE NO OUTPUT DATA */
#define CMD1H_CLR_STALL 0x41 /* Host Mode: Control Transport - Clear Endpoint Errors */
/* input: endpoint number */
/* output interrupt */
#define CMD1H_SET_ADDRESS 0x45 /* Host Mode: Control Transfer - Set USB Address */
/* enter: address value */
/* output interrupt */
#define CMD1H_GET_DESCR 0x46 /* Host Mode: Control Transfer - Get Descriptor */
/* input: descriptor type */
/* output interrupt */
#define CMD1H_SET_CONFIG 0x49 /* Host Mode: Control Transfer - Set UP USB Configuration */
/* enter: configuration value */
/* output interrupt */
#define CMD0H_AUTO_SETUP 0x4D /* Host mode/SD card not supported: Automatic configuration of USB devices */
/* output interrupt */
#define CMD2H_ISSUE_TKN_X 0x4E /* Host mode: Issue a synchronization token, execute the transaction, this command can be used instead of CMD10_SET_ENDP6/CMD10_SET_ENDP7 + CMD1H_ISSUE_TOKEN */
/* input: synchronization flag, transaction properties */
/* BIT 7 OF THE SYNC FLAG IS THE SYNC TRIGGER BIT OF THE HOST ENDPOINT IN, BIT 6 IS THE SYNC TRIGGER BIT OF THE HOST ENDPOINT OUT, BITS 5 TO BIT 0 MUST BE 0 */
/* the lower 4 bits of the transaction property are tokens, the high 4 bits are the endpoint number */
/* Output interrupt */
#define CMD1H_ISSUE_TOKEN 0x4F /* Host mode: issue token, execute transaction, suggest to use CMD2H_ISSUE_TKN_X command  */
/* Input: Transaction attributes  */
/*           The lower 4 bits are the token, and the upper 4 bits are the endpoint number  */
/* Output interrupt  */
#define CMD0H_DISK_INIT 0x51 /* Host mode/SD card not supported: Initialize USB memory  */
/* Output interrupt  */
#define CMD0H_DISK_RESET 0x52 /* Host mode/SD card not supported: control transfer-reset USB memory  */
/* Output interrupt  */
#define CMD0H_DISK_SIZE 0x53 /* Host mode/SD card not supported: Get the capacity of the USB memory  */
/* Output interrupt  */
#define CMD0H_DISK_INQUIRY 0x58 /* Host mode/SD card not supported: Query USB memory characteristics  */
/* Output interrupt  */
#define CMD0H_DISK_READY 0x59 /* Host mode/SD card not supported: Check that the USB storage is ready  */
/* Output interrupt  */
#define CMD0H_DISK_R_SENSE 0x5A /* Host mode/SD card not supported: Check for USB storage errors  */
/* Output interrupt  */
#define CMD0H_RD_DISK_SEC 0x5B /* Host file mode: read a sector of data from the disk to the internal buffer  */
/* Output interrupt  */
#define CMD0H_WR_DISK_SEC 0x5C /* Hosts file mode: Writes data from one sector of the internal buffer to disk */
/* output interrupt */
#define CMD0H_DISK_MAX_LUN 0x5D /* Host mode: Control transfer - Get the maximum logical unit number of USB memory */
/* output interrupt */
/* ********************************************************************************************************************* */
/* THE FOLLOWING DEFINITIONS ARE FOR THE PURPOSE OF COMPATIBILITY WITH THE COMMAND NAME FORMAT IN CH375 INCLUDE FILES */
#ifndef _NO_CH375_COMPATIBLE_
#define CMD_GET_IC_VER CMD01_GET_IC_VER
#define CMD_SET_BAUDRATE CMD21_SET_BAUDRATE
#define CMD_ENTER_SLEEP CMD00_ENTER_SLEEP
#define CMD_RESET_ALL CMD00_RESET_ALL
#define CMD_CHECK_EXIST CMD11_CHECK_EXIST
#define CMD_CHK_SUSPEND CMD20_CHK_SUSPEND
#define CMD_SET_SDO_INT CMD20_SET_SDO_INT
#define CMD_GET_FILE_SIZE CMD14_GET_FILE_SIZE
#define CMD_SET_FILE_SIZE CMD50_SET_FILE_SIZE
#define CMD_SET_USB_MODE CMD11_SET_USB_MODE
#define CMD_GET_STATUS CMD01_GET_STATUS
#define CMD_UNLOCK_USB CMD00_UNLOCK_USB
#define CMD_RD_USB_DATA0 CMD01_RD_USB_DATA0
#define CMD_RD_USB_DATA CMD01_RD_USB_DATA
#define CMD_WR_USB_DATA7 CMD10_WR_USB_DATA7
#define CMD_WR_HOST_DATA CMD10_WR_HOST_DATA
#define CMD_WR_REQ_DATA CMD01_WR_REQ_DATA
#define CMD_WR_OFS_DATA CMD20_WR_OFS_DATA
#define CMD_SET_FILE_NAME CMD10_SET_FILE_NAME
#define CMD_DISK_CONNECT CMD0H_DISK_CONNECT
#define CMD_DISK_MOUNT CMD0H_DISK_MOUNT
#define CMD_FILE_OPEN CMD0H_FILE_OPEN
#define CMD_FILE_ENUM_GO CMD0H_FILE_ENUM_GO
#define CMD_FILE_CREATE CMD0H_FILE_CREATE
#define CMD_FILE_ERASE CMD0H_FILE_ERASE
#define CMD_FILE_CLOSE CMD1H_FILE_CLOSE
#define CMD_DIR_INFO_READ CMD1H_DIR_INFO_READ
#define CMD_DIR_INFO_SAVE CMD0H_DIR_INFO_SAVE
#define CMD_BYTE_LOCATE CMD4H_BYTE_LOCATE
#define CMD_BYTE_READ CMD2H_BYTE_READ
#define CMD_BYTE_RD_GO CMD0H_BYTE_RD_GO
#define CMD_BYTE_WRITE CMD2H_BYTE_WRITE
#define CMD_BYTE_WR_GO CMD0H_BYTE_WR_GO
#define CMD_DISK_CAPACITY CMD0H_DISK_CAPACITY
#define CMD_DISK_QUERY CMD0H_DISK_QUERY
#define CMD_DIR_CREATE CMD0H_DIR_CREATE
#define CMD_SEC_LOCATE CMD4H_SEC_LOCATE
#define CMD_SEC_READ CMD1H_SEC_READ
#define CMD_SEC_WRITE CMD1H_SEC_WRITE
#define CMD_DISK_BOC_CMD CMD0H_DISK_BOC_CMD
#define CMD_DISK_READ CMD5H_DISK_READ
#define CMD_DISK_RD_GO CMD0H_DISK_RD_GO
#define CMD_DISK_WRITE CMD5H_DISK_WRITE
#define CMD_DISK_WR_GO CMD0H_DISK_WR_GO
#define CMD_SET_USB_SPEED CMD10_SET_USB_SPEED
#define CMD_GET_DEV_RATE CMD11_GET_DEV_RATE
#define CMD_GET_TOGGLE CMD11_GET_TOGGLE
#define CMD_READ_VAR8 CMD11_READ_VAR8
#define CMD_SET_RETRY CMD20_SET_RETRY
#define CMD_WRITE_VAR8 CMD20_WRITE_VAR8
#define CMD_READ_VAR32 CMD14_READ_VAR32
#define CMD_WRITE_VAR32 CMD50_WRITE_VAR32
#define CMD_DELAY_100US CMD01_DELAY_100US
#define CMD_SET_USB_ID CMD40_SET_USB_ID
#define CMD_SET_USB_ADDR CMD10_SET_USB_ADDR
#define CMD_TEST_CONNECT CMD01_TEST_CONNECT
#define CMD_ABORT_NAK CMD00_ABORT_NAK
#define CMD_SET_ENDP2 CMD10_SET_ENDP2
#define CMD_SET_ENDP3 CMD10_SET_ENDP3
#define CMD_SET_ENDP4 CMD10_SET_ENDP4
#define CMD_SET_ENDP5 CMD10_SET_ENDP5
#define CMD_SET_ENDP6 CMD10_SET_ENDP6
#define CMD_SET_ENDP7 CMD10_SET_ENDP7
#define CMD_DIRTY_BUFFER CMD00_DIRTY_BUFFER
#define CMD_WR_USB_DATA3 CMD10_WR_USB_DATA3
#define CMD_WR_USB_DATA5 CMD10_WR_USB_DATA5
#define CMD_CLR_STALL CMD1H_CLR_STALL
#define CMD_SET_ADDRESS CMD1H_SET_ADDRESS
#define CMD_GET_DESCR CMD1H_GET_DESCR
#define CMD_SET_CONFIG CMD1H_SET_CONFIG
#define CMD_AUTO_SETUP CMD0H_AUTO_SETUP
#define CMD_ISSUE_TKN_X CMD2H_ISSUE_TKN_X
#define CMD_ISSUE_TOKEN CMD1H_ISSUE_TOKEN
#define CMD_DISK_INIT CMD0H_DISK_INIT
#define CMD_DISK_RESET CMD0H_DISK_RESET
#define CMD_DISK_SIZE CMD0H_DISK_SIZE
#define CMD_DISK_INQUIRY CMD0H_DISK_INQUIRY
#define CMD_DISK_READY CMD0H_DISK_READY
#define CMD_DISK_R_SENSE CMD0H_DISK_R_SENSE
#define CMD_RD_DISK_SEC CMD0H_RD_DISK_SEC
#define CMD_WR_DISK_SEC CMD0H_WR_DISK_SEC
#define CMD_DISK_MAX_LUN CMD0H_DISK_MAX_LUN
#endif
/* ********************************************************************************************************************* */
/* Parallel mode, bit definition of status port (read command port)  */
#ifndef PARA_STATE_INTB
#define PARA_STATE_INTB 0x80 /* Bit 7 of the parallel port mode status port: interrupt flag, active low  */
#define PARA_STATE_BUSY 0x10 /* Bit 4 of the parallel port mode status port: busy flag, active high  */
#endif
/* ********************************************************************************************************************* */
/* Serial port mode, boot synchronization code before operation command  */
#ifndef SER_CMD_TIMEOUT
#define SER_CMD_TIMEOUT 32 /* Serial command timeout time, the unit is mS, the interval between synchronization codes and between synchronization codes and command codes should be as short as possible, and the processing method after the timeout is discarded  */
#define SER_SYNC_CODE1 0x57 /* The first serial port synchronization code to start the operation  */
#define SER_SYNC_CODE2 0xAB /* The second serial port synchronization code to start the operation  */
#endif
/* ********************************************************************************************************************* */
/* Operating status  */
#ifndef CMD_RET_SUCCESS
#define CMD_RET_SUCCESS 0x51 /* Command operation succeeded  */
#define CMD_RET_ABORT 0x5F /* Command operation failed  */
#endif
/* ********************************************************************************************************************* */
/* USB interrupt status  */
#ifndef USB_INT_EP0_SETUP
/* The following status codes are special event interrupts. If the USB bus suspension check is enabled through CMD20_CHK_SUSPEND, then the interrupt status of USB bus suspension and sleep wakeup must be handled  */
#define USB_INT_USB_SUSPEND 0x05 /* USB bus hang event  */
#define USB_INT_WAKE_UP 0x06 /* Wake-up event from sleep  */
/* The following status code 0XH is used for USB device mode  */
/*   Only need to process in the built-in firmware mode : USB_INT_EP1_OUT, USB_INT_EP1_IN, USB_INT_EP2_OUT, USB_INT_EP2_IN */
/*   Bit 7-Bit 4 is 0000  */
/*   Bit 3-Bit 2 indicate the current transaction , 00=OUT, 10=IN, 11=SETUP */
/*   Bit 1-Bit 0 indicates the current endpoint, 00=Endpoint 0, 01=Endpoint 1, 10=Endpoint 2, 11=USB bus reset  */
#define USB_INT_EP0_SETUP 0x0C /* SETUP of USB endpoint 0  */
#define USB_INT_EP0_OUT 0x00 /* OUT of USB endpoint 0  */
#define USB_INT_EP0_IN 0x08 /* IN of USB endpoint 0  */
#define USB_INT_EP1_OUT 0x01 /* OUT of USB endpoint 1  */
#define USB_INT_EP1_IN 0x09 /* IN of USB endpoint 1  */
#define USB_INT_EP2_OUT 0x02 /* OUT of USB endpoint 2  */
#define USB_INT_EP2_IN 0x0A /* IN of USB Endpoint 2 */
/* USB_INT_BUS_RESET 0x0000XX11B */ /* USB bus reset */
#define USB_INT_BUS_RESET1 0x03 /* USB bus reset */
#define USB_INT_BUS_RESET2 0x07 /* USB bus reset */
#define USB_INT_BUS_RESET3 0x0B /* USB bus reset */
#define USB_INT_BUS_RESET4 0x0F /* USB bus reset */
#endif
/* THE FOLLOWING STATUS CODES 2XH-3XH COMMUNICATION FAILURE CODES FOR USB HOST MODE */
/* bit 7-bit 6 is 00 */
/* bit 5 to 1 */
/* bit 4 indicates whether the currently received packet is synchronized */
/* BIT 3-BIT 0 INDICATES THE ANSWER OF THE USB DEVICE WHEN COMMUNICATION FAILS: 0010=ACK, 1010=NAK, 1110=STALL, 0011=DATA0, 1011=DATA1, XX00=TIMEOUT */
/* USB_INT_RET_ACK 0x001X0010B */ /* Error: ACK returned for IN transaction */
/* USB_INT_RET_NAK 0x001X1010B */ /* Error: NAK returned */
/* USB_INT_RET_STALL 0x001X1110B */ /* Error: CALL returned */
	/* USB_INT_RET_DATA0 0x001X0011B */ /* Error: DATA0  returned for OUT / SETUP transactions */
		/* USB_INT_RET_DATA1 0x001X1011B */ /* Error: DATA1  returned for OUT / SETUP transactions */
		/* USB_INT_RET_TOUT 0x001XXX00B */ /* Error: Timeout Returned */
		/* USB_INT_RET_TOGX 0x0010X011B */ /* Error: Data returned for IN transactions is not synchronized */
		/* USB_INT_RET_PID 0x001XXXXXB */ /* Error: Not defined */
		/* THE FOLLOWING STATUS CODE 1XH IS USED FOR OPERATION STATUS CODES IN USB HOST MODE */
#ifndef USB_INT_SUCCESS
#define USB_INT_SUCCESS 0x14 /* USB transaction or transfer operation successful */
#define USB_INT_CONNECT 0x15 /* USB device connection event detected, may be a new connection or disconnected and reconnected */
#define USB_INT_DISCONNECT 0x16 /* USB device disconnect event detected */
#define USB_INT_BUF_OVER 0x17 /* Incorrect data transferred from USB or too much data buffer overflow */
#define USB_INT_USB_READY 0x18 /* USB device has been initialized (USB address has been assigned) */
#define USB_INT_DISK_READ 0x1D /* USB memory request data readout */
#define USB_INT_DISK_WRITE 0x1E /* USB memory request data write */
#define USB_INT_DISK_ERR 0x1F /* USB memory operation failed */
#endif
/* the following status codes are used for file system error codes in host file mode */
#ifndef ERR_DISK_DISCON
#define ERR_DISK_DISCON 0x82 /* Disk is not connected, it may have been disconnected */
#define ERR_LARGE_SECTOR 0x84 /* The sectors of the disk are too large and only support 512 bytes per sector */
#define ERR_TYPE_ERROR 0x92 /* Disk partition type is not supported, only FAT12/FAT16/BigDOS/FAT32 is supported and needs to be repartitioned by Disk Management Tool */
#define ERR_BPB_ERROR 0xA1 /* Disk has not been formatted, or the parameters are wrong and need to be reformatted by WINDOWS with default parameters */
#define ERR_DISK_FULL 0xB1 /* Disk files are too full, have too little or no space left, need to be defragmented */
#define ERR_FDT_OVER 0xB2 /* There are too many files in the directory (folder), there are no free directory entries, the number of files under the FAT12/FAT16 root directory should be less than 512, and disking is required */
#define ERR_FILE_CLOSE 0xB4 /* File is closed, you should reopen it if you need to use it */
#define ERR_OPEN_DIR 0x41 /* The directory (folder) with the specified path is opened */
#define ERR_MISS_FILE 0x42 /* The file with the specified path was not found, it may be that the file name is wrong */
#define ERR_FOUND_NAME 0x43 /* search for a matching file name, or ask to open a directory (folder) and the actual result is that the file is open */
/* the following file system error codes are used for file system subroutines */
#define ERR_MISS_DIR 0xB3 /* A subdirectory (folder) of the specified path is not found, it may be that the directory name is wrong */
#define ERR_LONG_BUF_OVER 0x48 /* Long file buffer overflow */
#define ERR_LONG_NAME_ERR 0x49 /* Short file name does not have a corresponding long file name or long file name error */
#define ERR_NAME_EXIST 0x4A /* A short file with the same name already exists, it is recommended to regenerate another short file name */
#endif
/* ********************************************************************************************************************* */
/* THE FOLLOWING STATUS CODES ARE USED FOR DISK AND FILE STATUS IN HOST FILE MODE, VAR_DISK_STATUS */
#ifndef DEF_DISK_UNKNOWN
#define DEF_DISK_UNKNOWN 0x00 /* Not initialized, unknown status */
#define DEF_DISK_DISCONN 0x01 /* Disk is not connected or has been disconnected */
#define DEF_DISK_CONNECT 0x02 /* Disk is already attached, but it has not been initialized or is not recognized */
#define DEF_DISK_MOUNTED 0x03 /* Disk has been initialized successfully, but the file system has not been parsed or the file system does not support */
#define DEF_DISK_READY 0x10 /* The file system of the disk has been analyzed and is able to support */
#define DEF_DISK_OPEN_ROOT 0x12 /* The root directory has been opened, it must be closed after use, note that the FAT12/FAT16 root directory is a fixed length */
#define DEF_DISK_OPEN_DIR 0x13 /* Subdirectories (folders) have been opened */
#define DEF_DISK_OPEN_FILE 0x14 /* File Already Open */
#endif
/* ********************************************************************************************************************* */
/* common definitions of file systems */
#ifndef DEF_SECTOR_SIZE
#define DEF_SECTOR_SIZE 512 /* The default size of the physical sector of the USB flash drive or SD card */
#endif
#ifndef DEF_CURSOR_END
#define DEF_CURSOR_END 0xFFFFFFFF /* The default size of the physical sector of the USB flash drive or SD card */
#endif
#ifndef DEF_CURSOR_BEGIN
#define DEF_CURSOR_BEGIN 0x00000000 /* The default size of the physical sector of the USB flash drive or SD card */
#endif
#ifndef DEF_WILDCARD_CHAR
#define DEF_WILDCARD_CHAR 0x2A /* Wildcard character for pathname '*' */
#define DEF_SEPAR_CHAR1 0x5C /* pathname '' */
#define DEF_SEPAR_CHAR2 0x2F /* pathname '/' */
#define DEF_FILE_YEAR 2004 /* Default file date: 2004 */
#define DEF_FILE_MONTH 1 /* Default file date: January */
#define DEF_FILE_DATE 1 /* Default file date: 1st */
#endif
#ifndef ATTR_DIRECTORY
	/* Returns: Enumerated file directory information */
	/* CMD0H_FILE_ENUM_GO: CONTINUE ENUMERATING FILES AND DIRECTORIES (FOLDERS) */
	/* CMD0H_FILE_OPEN: Enumerate files and directories (folders) */
	typedef struct _FAT_DIR_INFO {
		char DIR_Name[11]; /* 00H, file name, 11 bytes total, blank spaces */
		UINT8 DIR_Attr; /* 0BH, file attributes, refer to the instructions that follow */
		UINT8 DIR_NTRes; /* 0CH */
		UINT8 DIR_CrtTimeTenth; /* 0DH, time of file creation, counted in 0.1 seconds */
		UINT16 DIR_CrtTime; /* 0EH, time the file was created */
		UINT16 DIR_CrtDate; /* 10H, date the file was created */
		UINT16 DIR_LstAccDate; /* 12H, date of the most recent access operation */
		UINT16 DIR_FstClusHI; /* 2PM */
		UINT16 DIR_WrtTime; /* 16H, file modification time, refer to the previous macro MAKE_FILE_TIME */
		UINT16 DIR_WrtDate; /* 18H, date the file was modified, refer to the previous macro MAKE_FILE_DATE */
		UINT16 DIR_FstClusLO; /* 1AH */
		UINT32 DIR_FileSize; /* 1CH, file length */
	} FAT_DIR_INFO, * P_FAT_DIR_INFO; /* 20H */
	/* file attributes */
#define ATTR_READ_ONLY 0x01 /* File is read-only */
#define ATTR_HIDDEN 0x02 /* File is an implicit attribute */
#define ATTR_SYSTEM 0x04 /* File is a system property */
#define ATTR_VOLUME_ID 0x08 /* Label */
#define ATTR_DIRECTORY 0x10 /* Subdirectories (folders) */
#define ATTR_ARCHIVE 0x20 /* File is an archive attribute */
#define ATTR_LONG_NAME ( ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID ) /* Long file name attribute */
#define ATTR_LONG_NAME_MASK ( ATTR_LONG_NAME | ATTR_DIRECTORY | ATTR_ARCHIVE )
/* FILE ATTRIBUTES UINT8 */
/* bit0 bit1 bit2 bit3 bit4 bit5 bit6 bit7 */
/* implicit volume only undefined */
/* read tibetan standards */
/* FILE TIME UINT16 */
/* Time = (Hour<<11) + (Minute<<5) + (Second>>1) */
#define MAKE_FILE_TIME( h, m, s ) ( (h<<11) + (m<<5) + (s>>1) / * Generate file time data for the specified time, minute, and second */
/* FILE DATE UINT16 */
/* Date = ((Year-1980)<<9) + (Month<<5) + Day */
#define MAKE_FILE_DATE( y, m, d ) ( ((y-1980) <<9) + (m<<5) + d ) / * Generate date data for the specified year, month and day */
#define LONE_NAME_MAX_CHAR (255*2) /* Maximum number of characters/bytes for long file names */
#define LONG_NAME_PER_DIR (13*2) /* Long filenames number of characters/bytes in each file directory information structure */
#endif
/* ********************************************************************************************************************* */
/* SCSI COMMAND AND DATA INPUT AND OUTPUT STRUCTURE */
#ifndef SPC_CMD_INQUIRY
/* SCSI COMMAND CODE */
#define SPC_CMD_INQUIRY 0x12
#define SPC_CMD_READ_CAPACITY 0x25
#define SPC_CMD_READ10 0x28
#define SPC_CMD_WRITE10 0x2A
#define SPC_CMD_TEST_READY 0x00
#define SPC_CMD_REQUEST_SENSE 0x03
#define SPC_CMD_MODESENSE6 0x1A
#define SPC_CMD_MODESENSE10 0x5A
#define SPC_CMD_START_STOP 0x1B
	/* BulkOnly Protocol Command Blocks */
	/* Input parameters: CBW command structure */
	/* CMD0H_DISK_BOC_CMD: Command to execute the BulkOnly Transfer Protocol on USB memory */
	typedef struct _BULK_ONLY_CBW {
		UINT32 CBW_Sig;
		UINT32 CBW_Tag;
		UINT8 CBW_DataLen0; /* 08H, input: data transfer length, valid values from 0 to 48 for input data, 0 to 33 for output data */
		UINT8 CBW_DataLen1;
		UINT16 CBW_DataLen2;
		UINT8 CBW_Flag; /* 0CH, input: transmission direction and other flags, bit 7 is 1 to input data, bit 0 is output data or no data */
		UINT8 CBW_LUN;
		UINT8 CBW_CB_Len; /* 0EH, enter: length of the command block, valid values are 1 to 16 */
		UINT8 CBW_CB_Buf[16]; /* 0FH, enter: command block, the buffer can be up to 16 bytes */
	} BULK_ONLY_CBW, * P_BULK_ONLY_CBW; /* BulkOnly protocol command block, enter cbw structure */
	/* Return: Data returned by the INQUIRY command */
	/* CMD0H_DISK_MOUNT: Initialize the disk and test whether the disk is ready, the first time it is executed  */
	/* CMD0H_DISK_INQUIRY: QUERY USB MEMORY CHARACTERISTICS */
	/* CMD0H_DISK_INIT: INITIALIZE THE USB MEMORY */
	typedef struct _INQUIRY_DATA {
		UINT8 DeviceType; /* 00H, Device Type */
		UINT8 RemovableMedia; /* 01H, bit 7 for 1 description is removable storage */
		UINT8 Versions; /* 02H, Protocol Version */
		UINT8 DataFormatAndEtc; /* 03H, specify the return data format */
		UINT8 AdditionalLength; /* 04H, length of subsequent data */
		UINT8 Reserved1;
		UINT8 Reserved2;
		UINT8 MiscFlag; /* 07H, some control flags */
		UINT8 VendorIdStr[8]; /* 08H, Vendor Information */
		UINT8 ProductIdStr[16]; /* 10H, Product Information */
		UINT8 ProductRevStr[4]; /* 20H, Product Version */
	} INQUIRY_DATA, * P_INQUIRY_DATA; /* 24H */
	/* Return: Data returned by the REQUEST SENSE command */
	/* CMD0H_DISK_R_SENSE: CHECK FOR USB MEMORY ERRORS */
	typedef struct _SENSE_DATA {
		UINT8 ErrorCode; /* 00H, Error code and valid bit  */
		UINT8 SegmentNumber;
		UINT8 SenseKeyAndEtc; /* 02H, Primary key code  */
		UINT8 Information0;
		UINT8 Information1;
		UINT8 Information2;
		UINT8 Information3;
		UINT8 AdditSenseLen; /* 07H, Length of subsequent data  */
		UINT8 CmdSpecInfo[4];
		UINT8 AdditSenseCode; /* 0CH, Additional key code  */
		UINT8 AddSenCodeQual; /* 0DH, Detailed additional key codes  */
		UINT8 FieldReplaUnit;
		UINT8 SenseKeySpec[3];
	} SENSE_DATA, * P_SENSE_DATA; /* 12H */
#endif
/* ********************************************************************************************************************* */
/* Data input and output structure in host file mode  */
#ifndef MAX_FILE_NAME_LEN
#define MAX_FILE_NAME_LEN (13+1) /* The maximum length of the file name, the maximum length is 1 root directory character + 8 main file names + 1 decimal point + 3 type names + terminator = 14  */
	/* Command input data and output data  */
	typedef struct {
		UINT8 mBuffer[MAX_FILE_NAME_LEN];
	} Default;
	typedef struct {
		UINT8 mUpdateFileSz; /* Input parameters: whether to allow update file length, 0 to disable update length */
	} FileCLose; /* CMD1H_FILE_CLOSE: Closes the currently open file */
	typedef struct {
		UINT8 mDirInfoIndex; /* Input parameters: specify the index number of the directory information structure to be read in the sector, 0FFH is the currently open file */
	} DirInfoRead; /* CMD1H_DIR_INFO_READ: Read the directory information of the file */
	typedef struct {
		uint8_t mByte[4]; /* Input parameters: Number of offset bytes, offset in bytes (total length 32 bits, low bytes first) */
		UINT32 mSectorLba; /* Returns: The absolute linear sector number corresponding to the current file pointer, 0FFFFFFFFH has reached the end of the file (total length 32 bits, low bytes before) */
	} ByteLocate; /* CMD4H_BYTE_LOCATE: Moves the current file pointer in bytes */
	typedef struct {
		UINT16 mByteCount; /* Input parameters: The number of bytes requested to be read (total length 16 bits, low bytes first) */
	} ByteRead; /* CMD2H_BYTE_READ: Reads a block of data from the current position in bytes */
	typedef struct {
		UINT16 mByteCount; /* Input parameters: The number of bytes requested to write (total length 16 bits, low bytes first) */
	} ByteWrite; /* CMD2H_BYTE_WRITE: Writes a block in bytes to the current location */
	typedef union {
		UINT32 mSectorOffset; /* Input parameters: Number of offset sectors, offset in sectors (total length 32 bits, low bytes first) */
		UINT32 mSectorLba; /* Returns: The absolute linear sector number corresponding to the current file pointer, 0FFFFFFFFH has reached the end of the file (total length 32 bits, low bytes before) */
	} SectorLocate; /* CMD4H_SEC_LOCATE: Moves the current file pointer in sectors */
	typedef struct {
		UINT8 mSectorCount; /* Input parameter: Number of sectors requested to be read */
		/* return: number of sectors allowed to be read */
		UINT8 mReserved1;
		UINT8 mReserved2;
		UINT8 mReserved3;
		UINT32 mStartSector; /* Return: The starting absolute linear sector number of the sector block allowed to be read (total length 32 bits, low bytes before) */
	} SectorRead; /* CMD1H_SEC_READ: Reads blocks from the current position in sectors */
	typedef struct {
		UINT8 mSectorCount; /* Input parameter: Number of sectors written to the request */
		/* return: number of sectors allowed to write */
		UINT8 mReserved1;
		UINT8 mReserved2;
		UINT8 mReserved3;
		UINT32 mStartSector; /* Return: The starting absolute linear sector number of the sector block allowed to be written (total length 32 bits, low bytes before) */
	} SectorWrite; /* CMD1H_SEC_WRITE: Writes blocks in sectors at the current location */
	typedef struct {
		UINT32 mDiskSizeSec; /* Returned: The total number of sectors of the entire physical disk (total length 32 bits, low bytes in front) */
	} DiskCapacity; /* CMD0H_DISK_CAPACITY: Query disk physical capacity */
	typedef struct {
		UINT32 mTotalSector; /* Returns: The total number of sectors of the current logical disk (total length 32 bits, low bytes before) */
		UINT32 mFreeSector; /* Returns: The number of sectors remaining on the current logical disk (total length 32 bits, low bytes in front) */
		UINT8 mDiskFat; /* Returns: FAT type of the current logical disk, 1-FAT12, 2-FAT16, 3-FAT32 */
	} DiskQuery; /* CMD_DiskQuery, Query disk information */
	typedef struct {
		UINT8 mMaxLogicUnit; /* Returns: Maximum logical unit number for USB memory */
	} DiskMaxLun; /* CMD0H_DISK_MAX_LUN: Control Transfer - Gets Maximum Logical Unit Number for USB Memory */
	typedef struct {
		UINT32 mDiskSizeSec; /* Returns: Total number of sectors for the entire physical disk (total length 32 bits, high bytes in front) */
	} DiskSize; /* CMD0H_DISK_SIZE: Get the capacity of the USB memory */
	typedef struct {
		UINT32 mStartSector; /* Input parameters: LBA sector address (total length 32 bits, low bytes first) */
		UINT8 mSectorCount; /* Input parameter: Number of sectors requested to be read */
	} DiskRead; /* CMD5H_DISK_READ: Read data blocks (in sectors) from USB memory */
	typedef struct {
		UINT32 mStartSector; /* Input parameters: LBA sector address (total length 32 bits, low bytes first) */
		UINT8 mSectorCount; /* Input parameter: Number of sectors written to the request */
	} DiskWrite; /* CMD5H_DISK_WRITE: Write blocks (in sectors) to USB memory */
#endif
	/* ********************************************************************************************************************* */
	/* address of file system variable in host file mode */
#ifndef VAR_FILE_SIZE
/* 8 bits/single-byte variable */
#define VAR_SYS_BASE_INFO 0x20 /* Basic information about the current system */
/* Bit 6 is used to indicate subcategory SubClass-Code for USB storage devices, bit 6 for 0 indicates subcategory 6, bit 6 for 1 indicates that subcategory is another value of non-6 */
/* BIT 5 IS USED TO INDICATE USB CONFIGURATION STATUS IN USB DEVICE MODE AND USB DEVICE CONNECTION STATUS IN USB HOST MODE */
/* IN USB DEVICE MODE, BIT 5 IS 1, THE USB CONFIGURATION IS COMPLETE, AND BIT 5 BIT 0 HAS NOT BEEN CONFIGURED */
/* IN USB HOST MODE, BIT 5 IS 1 IF THE USB PORT HAS A USB DEVICE, AND BIT 5 IS 0 IF THE USB PORT HAS NO USB DEVICE */
	/*BIT 4 OF INDICATES THE BUFFER LOCK STATUS OF USB DEVICE MODE, BIT 4 IS 1 TO INDICATE THAT THE USB BUFFER IS LOCKED, AND BIT 6 IS 1 TO INDICATE THAT IT HAS BEEN FREED */
	/* other bits, reserved, do not modify */
#define VAR_RETRY_TIMES 0x25 /* Number of retries for USB transaction operations */
/* BIT 7 IS 0 AND DOES NOT RETRY WHEN RECEIVING NAK, BIT 7 IS 1 BIT 6 IS 0 AND IS UNLIMITED RETRY WHEN NAK IS RECEIVED (YOU CAN GIVE UP RETRY WITH THE CMD_ABORT_NAK COMMAND), BIT 7 IS 1 BIT 6 IS 1 AND RETRYS FOR UP TO 3 SECONDS WHEN YOU RECEIVE NAK */
/* bits 5 to bit 0 are the number of retries after the timeout */
#define VAR_FILE_BIT_FLAG 0x26 /* Bit flags in hosts file mode */
/* BIT 1 AND BIT 0, FAT FILE SYSTEM FLAGS FOR LOGICAL DISKS, 00-FAT12, 01-FAT16, 10-FAT32, 11-ILLEGAL */
/* BIT 2, FLAG WHETHER FAT TABLE DATA IN THE CURRENT BUFFER HAS BEEN MODIFIED, 0-UNMODIFIED, 1-MODIFIED */
/* bit 3, file length needs to be modified flag, current file is appended with data, 0-no appended without modification, 1- appended needs to be modified */
/* other bits, reserved, do not modify */
#define VAR_DISK_STATUS 0x2B /* Disk and file status in Hosts File mode */
#define VAR_SD_BIT_FLAG 0x30 /* Bit flag of SD card in host file mode */
/* BIT 0, SD CARD VERSION, 0- ONLY SD FIRST VERSION SUPPORTED, 1- SUPPORT SD SECOND VERSION */
/* BIT 1, AUTO IDENTIFY, 0-SD CARD, 1-MMC CARD */
/* BIT 2, AUTO IDENTIFY, 0-STANDARD CAPACITY SD CARD, 1-HIGH CAPACITY SD CARD (HC-SD) */
/* BIT 4, ACMD41 COMMAND TIMED OUT */
/* BIT 5, CMD1 COMMAND TIMED OUT */
/* BIT 6, CMD58 COMMAND TIMED OUT */
/* other bits, reserved, do not modify */
/* synchronization flag for #define VAR_UDISK_TOGGLE 0x31  BUNK-IN/BULK-OUT endpoints for USB storage devices */
/* Bit 7, Bulk-In Endpoint Synchronization Flag */
/* Bit 6, Bulk-In Endpoint Synchronization Flag */
/* bits 5~bit 0, must be 0 */
#define VAR_UDISK_LUN 0x34 /* Logical unit number of the USB storage device */
/* BITS 7~BIT 4, THE CURRENT LOGICAL UNIT NUMBER OF THE USB STORAGE DEVICE, CH376 AFTER INITIALIZING THE USB STORAGE DEVICE, THE DEFAULT IS TO ACCESS THE 0# LOGICAL UNIT */
/* BITS 3 TO 0, THE MAXIMUM LOGICAL UNIT NUMBER OF THE USB STORAGE DEVICE, PLUS 1 EQUALS THE NUMBER OF LOGICAL UNITS */
#define VAR_SEC_PER_CLUS 0x38 /* Number of sectors per logical disk */
#define VAR_FILE_DIR_INDEX 0x3B /* The index number of the current file directory information in the sector */
#define VAR_CLUS_SEC_OFS 0x3C /* The current file pointer is offset by the sector within the cluster, or 0xFF points to the end of the file, and the cluster ends */
/* 32-bit/4-byte variable */
#define VAR_DISK_ROOT 0x44 /* For FAT16 disks is the number of sectors occupied by the root directory, for FAT32 disks is the root directory starting cluster number (total length 32 bits, low bytes in front) */
#define VAR_DSK_TOTAL_CLUS 0x48 /* Total number of clusters of logical disks (total length 32 bits, low bytes first) */
#define VAR_DSK_START_LBA 0x4C /* Logical disk starting absolute sector number LBA (total length 32 bits, low bytes in front) */
#define VAR_DSK_DAT_START 0x50 /* The starting LBA of the data region of the logical disk (total length 32 bits, low bytes first) */
#define VAR_LBA_BUFFER 0x54 /* LBA for the data corresponding to the current disk data buffer (total length 32 bits, low bytes first) */
#define VAR_LBA_CURRENT 0x58 /* Disk start LBA address for current reads and writes (total length 32 bits, low bytes first) */
#define VAR_FAT_DIR_LBA 0x5C /* LBA address of the sector where the current file directory information is located (total length 32 bits, low bytes first) */
#define VAR_START_CLUSTER 0x60 /* The starting cluster number of the current file or directory (folder) (total length 32 bits, low bytes first) */
#define VAR_CURRENT_CLUST 0x64 /* Current family number of the current file (total length 32 bits, low bytes first) */
#define VAR_FILE_SIZE 0x68 /* Length of the current file (total length 32 bits, low bytes first) */
#define VAR_CURRENT_OFFSET 0x6C /* Current file pointer, byte offset for current read and write position (total length 32 bits, low bytes before) */
#endif
/* ********************************************************************************************************************* */
/* COMMON USB DEFINITIONS */
/* USB PACKAGE IDENTIFICATION PID, HOST MODE MAY BE USED */
#ifndef DEF_USB_PID_SETUP
#define DEF_USB_PID_NULL 0x00 /* Retain PID, Undefined */
#define DEF_USB_PID_SOF 0x05
#define DEF_USB_PID_SETUP 0x0D
#define DEF_USB_PID_IN 0x09
#define DEF_USB_PID_OUT 0x01
#define DEF_USB_PID_ACK 0x02
#define DEF_USB_PID_NAK 0x0A
#define DEF_USB_PID_STALL 0x0E
#define DEF_USB_PID_DATA0 0x03
#define DEF_USB_PID_DATA1 0x0B
#define DEF_USB_PID_PRE 0x0C
#endif
/* USB REQUEST TYPE, EXTERNAL FIRMWARE MODE MAY BE USED */
#ifndef DEF_USB_REQ_TYPE
#define DEF_USB_REQ_READ 0x80 /* Control read operation */
#define DEF_USB_REQ_WRITE 0x00 /* Control write operations */
#define DEF_USB_REQ_TYPE 0x60 /* Control Request Type */
#define DEF_USB_REQ_STAND 0x00 /* Standard Request */
#define DEF_USB_REQ_CLASS 0x20 /* Device Class Request */
#define DEF_USB_REQ_VENDOR 0x40 /* Vendor Request */
#define DEF_USB_REQ_RESERVE 0x60 /* Retention Request */
#endif
/* USB standard device request, RequestType bit 6 bits 5=00 (Standard), external firmware mode may use */
#ifndef DEF_USB_GET_DESCR
#define DEF_USB_CLR_FEATURE 0x01
#define DEF_USB_SET_FEATURE 0x03
#define DEF_USB_GET_STATUS 0x00
#define DEF_USB_SET_ADDRESS 0x05
#define DEF_USB_GET_DESCR 0x06
#define DEF_USB_SET_DESCR 0x07
#define DEF_USB_GET_CONFIG 0x08
#define DEF_USB_SET_CONFIG 0x09
#define DEF_USB_GET_INTERF 0x0A
#define DEF_USB_SET_INTERF 0x0B
#define DEF_USB_SYNC_FRAME 0x0C
#endif
/* ********************************************************************************************************************* */
/* address of file system variable in usb host mode */
	typedef struct _USB_DEVICE_DESCRIPTOR {
		uint8_t bLength;
		uint8_t bDescriptorType;
		uint16_t bcdUSB;
		uint8_t bDeviceClass;
		uint8_t bDeviceSubClass;
		uint8_t bDeviceProtocol;
		uint8_t bMaxPacketSize0;
		uint16_t idVendor;
		uint16_t idProduct;
		uint16_t bcdDevice;
		uint8_t iManufacturer;
		uint8_t iProduct;
		uint8_t iSerialNumber;
		uint8_t bNumConfigurations;
	} USB_DEV_DESCR, * PUSB_DEV_DESCR;
	typedef struct _USB_CONFIG_DESCRIPTOR {
		uint8_t bLength;
		uint8_t bDescriptorType;
		uint16_t wTotalLength;
		uint8_t bNumInterfaces;
		uint8_t bConfigurationvalue;
		uint8_t iConfiguration;
		uint8_t bmAttributes;
		uint8_t bMaxPower;
	} USB_CFG_DESCR, * PUSB_CFG_DESCR;
	typedef struct _USB_INTERF_DESCRIPTOR {
		unsigned char bLength;
		unsigned char bDescriptorType;
		unsigned char bInterfaceNumber;
		unsigned char bAlternateSetting;
		unsigned char bNumEndpoints;
		unsigned char bInterfaceClass;
		unsigned char bInterfaceSubClass;
		unsigned char bInterfaceProtocol;
		unsigned char iInterface;
	} USB_ITF_DESCR, * PUSB_ITF_DESCR;
	typedef struct _USB_ENDPOINT_DESCRIPTOR {
		unsigned char bLength;
		unsigned char bDescriptorType;
		unsigned char bEndpointAddress;
		unsigned char bmAttributes;
		unsigned short wMaxPacketSize;
		unsigned char bInterval;
	} USB_ENDP_DESCR, * PUSB_ENDP_DESCR;
	typedef struct _USB_CONFIG_DESCRIPTOR_LONG {
		USB_CFG_DESCR cfg_descr;
		USB_ITF_DESCR itf_descr;
		USB_ENDP_DESCR endp_descr[4];
	} USB_CFG_DESCR_LONG, * PUSB_CFG_DESCR_LONG;
#pragma region enums
	enum BaudRate : uint32_t {
		BaudRate4800 = 4800,
		BaudRate9600 = 9600,
		BaudRate19200 = 19200,
		BaudRate38400 = 38400,
		BaudRate57600 = 57600,
		BaudRate115200 = 115200,
	};
	enum USB_SPEED : uint8_t {
		LOW_SPEED = 0x02,    //01: 1.5Mbps low-speed mode"
		HIGH_SPEED = 0x01,   //01: 1.5Mbps full speed mode (non-standard mode)"
		FULL_SPEED = 0x00    //00: 12Mbps full speed mode
	};
	enum USB_MODE : uint8_t {
		MODE_HOST_INV = 0x04,   //04: invalid usb host
		MODE_HOST_0 = 0x05,     //05: usb host, don't generate SOF
		MODE_HOST_1 = 0x07,     //07: usb host, bus reset
		MODE_HOST_2 = 0x06,     //06: usb host, generate SOF
		MODE_HOST_SD = 0x03,    //03: SD host, manage SD cards
		MODE_DEVICE_0 = 0x01,   //01: usb device, "peripheral firmware"
		MODE_DEVICE_1 = 0x02,   //02: usb device, "inner firmware"
		MODE_DEFAULT = 0x00     //00: invalid device mode (reset default)
	};
	enum fileProcessENUM { // for file read/write state machine
		REQUEST,
		NEXT,
		READWRITE,
		DONE
	};
#pragma endregion
	/* ********************************************************************************************************************* */
#ifdef __cplusplus
}
#endif
#endif