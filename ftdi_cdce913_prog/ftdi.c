#include <stdio.h>
#include <stdint.h>
#include "ftd2xx.h"
#include "ftdi.h"

#define false 0
#define true 1

#define I2C_OP_WRITE_BIT (0)
#define I2C_OP_READ_BIT  (1)

#define START_DURATION_1	10
#define START_DURATION_2	20

#define STOP_DURATION_1	10
#define STOP_DURATION_2	10
#define STOP_DURATION_3	10

#define SEND_ACK			0x00
#define SEND_NACK			0x80

#ifndef DEBUG
#define DEBUG 0
#endif


//------------------------------------------------------------------------------
// Open FTDI device + init mpsse
//------------------------------------------------------------------------------
FT_STATUS ftdi_open(FT_HANDLE *handle)
{
	FT_HANDLE ftHandle;
	FT_STATUS status;
	UCHAR bCommandEchod = false;
    DWORD dwCount;
    BYTE OutputBuffer[1024]; //Buffer to hold MPSSE commands and data to be sent to FT2232H
    BYTE InputBuffer[1024]; //Buffer to hold Data bytes to be read from FT2232H
    DWORD dwClockDivisor = 0x0095; //Value of clock divisor, SCL Frequency = 60/((1+0x0095)*2) (MHz) = 200khz
    DWORD dwNumBytesToSend = 0; //Index of output buffer
    DWORD dwNumBytesSent = 0, dwNumBytesRead = 0, dwNumInputBuffer = 0;

    //Try to open the FT2232H device port and get the valid handle for subsequent access
    char SerialNumBuf[64];

    status = FT_ListDevices((PVOID)0, &SerialNumBuf, FT_LIST_BY_INDEX | FT_OPEN_BY_SERIAL_NUMBER);
	if (status != FT_OK) {
		return status;
	}

    status = FT_OpenEx((PVOID) SerialNumBuf, FT_OPEN_BY_SERIAL_NUMBER, &ftHandle);
	if (status == FT_OK) { // Port opened successfully
		if (DEBUG) printf("FTDI Port opened successfully\n");

		status |= FT_ResetDevice(ftHandle); //Reset USB device

		//Purge USB receive buffer first by reading out all old data from FT2232H receive buffer
		status |= FT_GetQueueStatus(ftHandle, &dwNumInputBuffer); // Get the number of bytes in the FT2232H receive buffer
		if ((status == FT_OK) && (dwNumInputBuffer > 0)) {
			FT_Read(ftHandle, &InputBuffer, dwNumInputBuffer, &dwNumBytesRead); //Read out the data from FT2232H receive buffer
		}

		status |= FT_SetUSBParameters(ftHandle, 65536, 65535); //Set USB request transfer size
		status |= FT_SetChars(ftHandle, false, 0, false, 0); //Disable event and error characters
		status |= FT_SetTimeouts(ftHandle, 0, 5000); //Sets the read and write timeouts in milliseconds for the FT2232H
		status |= FT_SetLatencyTimer(ftHandle, 16); //Set the latency timer
		status |= FT_SetBitMode(ftHandle, 0x0, 0x00); //Reset controller
		status |= FT_SetBitMode(ftHandle, 0x0, 0x02); //Enable MPSSE mode
		if (status != FT_OK) { /*Error on initialize MPSEE of FT2232H*/
			if (DEBUG) printf("Error on initialize MPSEE of FT2232H\n");
			return status;
		}

		Sleep(50); // Wait for all the USB stuff to complete and work

		//////////////////////////////////////////////////////////////////
		// Below codes will synchronize the MPSSE interface by sending bad command ‘xAA’ and checking if the echo command followed by
		// bad command ‘AA’ can be received, this will make sure the MPSSE interface enabled and synchronized successfully
		//////////////////////////////////////////////////////////////////
		OutputBuffer[dwNumBytesToSend++] = '\xAA'; //Add BAD command ‘xAA’
		status = FT_Write(ftHandle, OutputBuffer, dwNumBytesToSend, &dwNumBytesSent); // Send off the BAD commands
		dwNumBytesToSend = 0; //Clear output buffer
		do {
			status = FT_GetQueueStatus(ftHandle, &dwNumInputBuffer); // Get the number of bytes in the device input buffer
		} while ((dwNumInputBuffer == 0) && (status == FT_OK)); //or Timeout

		bCommandEchod = false;

		status = FT_Read(ftHandle, &InputBuffer, dwNumInputBuffer, &dwNumBytesRead); //Read out the data from input buffer
		for (dwCount = 0; dwCount < dwNumBytesRead - 1; dwCount++) { //Check if Bad command and echo command received
			if ((InputBuffer[dwCount] == '\xFA') && (InputBuffer[dwCount+1] == '\xAA')) {
				bCommandEchod = true;
				break;
			}
		}
		if (bCommandEchod == false) { /*Error, can’t receive echo command , fail to synchronize MPSSE interface;*/ }

		////////////////////////////////////////////////////////////////////
		//Configure the MPSSE settings for I2C communication with 24LC256
		//////////////////////////////////////////////////////////////////
		OutputBuffer[dwNumBytesToSend++] = '\x8A'; //Ensure disable clock divide by 5 for 60Mhz master clock
		OutputBuffer[dwNumBytesToSend++] = '\x97'; //Ensure turn off adaptive clocking
		OutputBuffer[dwNumBytesToSend++] = '\x8C'; //Enable 3 phase data clock, used by I2C to allow data on both clock edges
		status = FT_Write(ftHandle, OutputBuffer, dwNumBytesToSend, &dwNumBytesSent); // Send off the commands

		dwNumBytesToSend = 0; //Clear output buffer

		OutputBuffer[dwNumBytesToSend++] = '\x80'; //Command to set directions of lower 8 pins and force value on bits set as output
		OutputBuffer[dwNumBytesToSend++] = '\x03'; //Set SDA, SCL high, WP disabled by SK, DO at bit ‘1’, GPIOL0 at bit ‘0’
		OutputBuffer[dwNumBytesToSend++] = '\x13'; //Set SK,DO,GPIOL0 pins as output with bit ’, other pins as input with bit ‘’

		// The SK clock frequency can be worked out by below algorithm with divide by 5 set as off
		// SK frequency = 60MHz /((1 + [(1 +0xValueH*256) OR 0xValueL])*2)
		OutputBuffer[dwNumBytesToSend++] = '\x86'; //Command to set clock divisor
		OutputBuffer[dwNumBytesToSend++] = dwClockDivisor & '\xFF'; //Set 0xValueL of clock divisor
		OutputBuffer[dwNumBytesToSend++] = (dwClockDivisor >> 8) & '\xFF'; //Set 0xValueH of clock divisor
		status = FT_Write(ftHandle, OutputBuffer, dwNumBytesToSend, &dwNumBytesSent); // Send off the commands

		dwNumBytesToSend = 0; //Clear output buffer

		Sleep(20); //Delay for a while

		//Turn off loop back in case
		OutputBuffer[dwNumBytesToSend++] = '\x85'; //Command to turn off loop back of TDI/TDO connection
		status = FT_Write(ftHandle, OutputBuffer, dwNumBytesToSend, &dwNumBytesSent); // Send off the commands
		dwNumBytesToSend = 0; //Clear output buffer

		Sleep(30); //Delay for a while
	}

	*handle = ftHandle;
	return status;
}

FT_STATUS I2C_Start(FT_HANDLE handle)
{
	FT_STATUS status;
	uint8_t buffer[(START_DURATION_1+START_DURATION_2+1)*3];
	uint32_t i = 0, j = 0;
	DWORD noOfBytesTransferred;

	/* SCL high, SDA high */
	for (j = 0; j < START_DURATION_1; j++)
	{
		buffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
		buffer[i++] = VALUE_SCLHIGH_SDAHIGH;
		buffer[i++] = DIRECTION_SCLOUT_SDAOUT;
	}
	/* SCL high, SDA low */
	for (j = 0; j < START_DURATION_2; j++)
	{
		buffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
		buffer[i++] = VALUE_SCLHIGH_SDALOW;
		buffer[i++] = DIRECTION_SCLOUT_SDAOUT;
	}
	/*SCL low, SDA low */
	buffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
	buffer[i++] = VALUE_SCLLOW_SDALOW;
	buffer[i++] = DIRECTION_SCLOUT_SDAOUT;

	status = FT_Write(handle, buffer, i, &noOfBytesTransferred);

	return status;
}

FT_STATUS I2C_Stop(FT_HANDLE handle)
{
	FT_STATUS status;
	uint8_t buffer[(STOP_DURATION_1+STOP_DURATION_2+STOP_DURATION_3+1)*3];
	uint32_t i = 0, j = 0;
	DWORD noOfBytesTransferred;

	/* SCL low, SDA low */
	for (j = 0; j < STOP_DURATION_1; j++)
	{
		buffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
		buffer[i++] = VALUE_SCLLOW_SDALOW;
		buffer[i++] = DIRECTION_SCLOUT_SDAOUT;
	}
	/* SCL high, SDA low */
	for (j = 0; j < STOP_DURATION_2; j++)
	{
		buffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
		buffer[i++] = VALUE_SCLHIGH_SDALOW;
		buffer[i++] = DIRECTION_SCLOUT_SDAOUT;
	}
	/* SCL high, SDA high */
	for (j = 0; j < STOP_DURATION_3; j++)
	{
		buffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
		buffer[i++] = VALUE_SCLHIGH_SDAHIGH;
		buffer[i++] = DIRECTION_SCLOUT_SDAOUT;
	}
	buffer[i++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
	buffer[i++] = VALUE_SCLHIGH_SDAHIGH;
	buffer[i++] = DIRECTION_SCLIN_SDAIN; /* Tristate the SCL & SDA pins */

	status = FT_Write(handle, buffer, i, &noOfBytesTransferred);

	return status;
}


































FT_STATUS I2C_Read8bitsAndGiveAck(FT_HANDLE handle, uint8_t *data, uint8_t ack)
{
	FT_STATUS status = FT_OTHER_ERROR;
	uint8_t buffer[20], inBuffer[5];
	uint32_t noOfBytes = 0;
	DWORD noOfBytesTransferred;

	/*set direction*/
	buffer[noOfBytes++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;/* MPSSE command */
	buffer[noOfBytes++] = VALUE_SCLLOW_SDALOW; /*Value*/
	buffer[noOfBytes++] = DIRECTION_SCLOUT_SDAIN; /*Direction*/

	/*Command to read 8 bits*/
	buffer[noOfBytes++] = MPSSE_CMD_DATA_IN_BITS_POS_EDGE;
	buffer[noOfBytes++] = DATA_SIZE_8BITS;/*0x00 = 1bit; 0x07 = 8bits*/
    /* Set directions to make SDA drive out. Pre-set state of SDA first though to avoid glitch */
	if (ack == 0) {
		/* We will drive the ACK bit to a '0' so pre-set pin to a '0' */
		buffer[noOfBytes++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
		buffer[noOfBytes++] = VALUE_SCLLOW_SDALOW;
		buffer[noOfBytes++] = DIRECTION_SCLOUT_SDAOUT;

		/* Clock out the ack bit as a '0' on negative edge */
		buffer[noOfBytes++] = MPSSE_CMD_DATA_OUT_BITS_NEG_EDGE;
		buffer[noOfBytes++] = DATA_SIZE_1BIT;
		buffer[noOfBytes++] = SEND_ACK;
	} else {
		/* We will release the ACK bit to a '1' so pre-set pin to a '1' by making it an input */
		buffer[noOfBytes++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
		buffer[noOfBytes++] = VALUE_SCLLOW_SDALOW;
		buffer[noOfBytes++] = DIRECTION_SCLOUT_SDAIN;

		/* Clock out the ack bit as a '1' on negative edge - never actually seen on line since SDA is input but burns off one bit time */
		buffer[noOfBytes++] = MPSSE_CMD_DATA_OUT_BITS_NEG_EDGE;
		buffer[noOfBytes++] = DATA_SIZE_1BIT;
		buffer[noOfBytes++] = SEND_NACK;
	}

	/* Back to Idle */
	buffer[noOfBytes++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;
	buffer[noOfBytes++] = VALUE_SCLLOW_SDALOW;
	buffer[noOfBytes++] = DIRECTION_SCLOUT_SDAIN;

	/* Command MPSSE to send data to PC immediately */
	buffer[noOfBytes++] = MPSSE_CMD_SEND_IMMEDIATE;

	status = FT_Write(handle, buffer, noOfBytes, &noOfBytesTransferred);
	if (FT_OK != status)
		return status;
	else if (noOfBytes != noOfBytesTransferred)	{
		if (DEBUG) printf("Requested to send %u bytes, no. of bytes sent is %u bytes\n",
			(unsigned)noOfBytes, (unsigned)noOfBytesTransferred);
		status = FT_IO_ERROR;
		return status;
	} else {
		noOfBytes = 1;
		status = FT_Read(handle, inBuffer, noOfBytes, &noOfBytesTransferred);
		if (FT_OK != status) {
			return status;
		} else if (noOfBytes != noOfBytesTransferred) {
			if (DEBUG) printf("Requested to read %u bytes, no. of bytes read is %u bytes\n",
				(unsigned)noOfBytes,(unsigned)noOfBytesTransferred);
			status = FT_IO_ERROR;
		} else {
			*data = inBuffer[0];
			if (DEBUG) printf("	*data = 0x%x\n", (unsigned)*data);
		}
	}

	return status;
}

FT_STATUS I2C_Write8bitsAndGetAck(FT_HANDLE handle, uint8_t data, uint8_t *ack)
{
	FT_STATUS status = FT_OTHER_ERROR;
	uint8_t buffer[20]={0};
	uint8_t inBuffer[3]={0};
	uint32_t noOfBytes = 0;
	DWORD noOfBytesTransferred;

	/*set direction*/
	buffer[noOfBytes++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;/* MPSSE command */
	buffer[noOfBytes++] = VALUE_SCLLOW_SDAHIGH; /*Value*/
	buffer[noOfBytes++] = DIRECTION_SCLOUT_SDAOUT; /*Direction*/

	/* Command to write 8bits */
	buffer[noOfBytes++]= MPSSE_CMD_DATA_OUT_BITS_NEG_EDGE;/* MPSSE command */
	buffer[noOfBytes++]= DATA_SIZE_8BITS;
	buffer[noOfBytes++] = data;

	/* Set SDA to input mode before reading ACK bit */
	buffer[noOfBytes++] = MPSSE_CMD_SET_DATA_BITS_LOWBYTE;/* MPSSE command */
	buffer[noOfBytes++] = VALUE_SCLLOW_SDALOW; /*Value*/
	buffer[noOfBytes++] = DIRECTION_SCLOUT_SDAIN; /*Direction*/

	/* Command to get ACK bit */
	buffer[noOfBytes++] = MPSSE_CMD_DATA_IN_BITS_POS_EDGE;/* MPSSE command */
	buffer[noOfBytes++] = DATA_SIZE_1BIT; /*Read only one bit */

	/*Command MPSSE to send data to PC immediately */
	buffer[noOfBytes++] = MPSSE_CMD_SEND_IMMEDIATE;
	status = FT_Write(handle, buffer, noOfBytes, &noOfBytesTransferred);

	if (DEBUG) printf("FT_Channel_Write returned: noOfBytes=%u, noOfBytesTransferred=%u \n",
		(unsigned)noOfBytes, (unsigned)noOfBytesTransferred);

	if (FT_OK != status) {
		if (DEBUG) printf("FT_OK != status \n");
		return status;
	} else if (noOfBytes != noOfBytesTransferred) {
		if (DEBUG) printf("Requested to send %u bytes, no. of bytes sent is %u bytes\n",
			(unsigned)noOfBytes, (unsigned)noOfBytesTransferred);
		status = FT_IO_ERROR;
		return status;
	} else { /*Get ack*/
		noOfBytes = 1;
		noOfBytesTransferred = 0;

		Sleep(1);
		status = FT_Read(handle, inBuffer, noOfBytes, &noOfBytesTransferred);

		if (DEBUG) printf("FT_Channel_Read returned: noOfBytes=%u, noOfBytesTransferred=%u\n",
			(unsigned)noOfBytes, (unsigned)noOfBytesTransferred);
		if (FT_OK != status) {
			return status;
		} else if (noOfBytes != noOfBytesTransferred) {
			if (DEBUG) printf("Requested to send %u bytes, no. of bytes sent is %u bytes\n",
				(unsigned)noOfBytes,(unsigned)noOfBytesTransferred);
			status = FT_IO_ERROR;
		} else {
			*ack = (uint8_t)(inBuffer[0] & 0x01);
			if (DEBUG) printf("success ACK= 0x%x; noOfBytes=%d noOfBytesTransferred=%d\n",
				(unsigned)*ack, (unsigned)noOfBytes, (unsigned)noOfBytesTransferred);
		}
	}

	return status;
}

FT_STATUS i2c_write_array(FT_HANDLE handle, const uint8_t i2c_dev_adr, const uint8_t *buf, const uint8_t buf_len)
{
	FT_STATUS status;
	uint8_t ack;
	uint32_t i=0;

	status = I2C_Start(handle);
	if (status != FT_OK) return status;

	status = I2C_Write8bitsAndGetAck(handle, (i2c_dev_adr << 1) | I2C_OP_WRITE_BIT, &ack);
	if (status) return status;

	if (ack) return FT_IO_ERROR;


	for (i=0; i<buf_len; i++) {
		status = I2C_Write8bitsAndGetAck(handle, buf[i], &ack);
        if (status) return status;
		if (ack) return FT_IO_ERROR;
	}

	status = I2C_Stop(handle);
	if (status != FT_OK) return status;

	return status;
}


FT_STATUS i2c_read_array(FT_HANDLE handle, const uint8_t i2c_dev_adr, uint8_t *buf, const uint8_t buf_len)
{
	FT_STATUS status;
	uint32_t i=0;
	uint8_t ack = 0;
	uint8_t d;

	//Purge USB receive buffer first before read operation
	//status = FT_GetQueueStatus(handle, &dwNumInputBuffer); // Get the number of bytes in the device receive buffer
	//if ((status == FT_OK) && (dwNumInputBuffer > 0)) {
	//	FT_Read(handle, &InputBuffer, dwNumInputBuffer, &dwNumBytesRead); //Read out all the data from receive buffer
	//}

	status = I2C_Start(handle);
	if (status) return status;

	status = I2C_Write8bitsAndGetAck(handle, (i2c_dev_adr << 1) | I2C_OP_READ_BIT, &ack);
	if (status) return status;
	if (ack) return FT_IO_ERROR;

	for (i=0; i<buf_len; i++) {
	    status = I2C_Read8bitsAndGiveAck(handle, &d, ack);
		buf[i] = d;
		if (status) return status;
	}

	status = I2C_Stop(handle);
	if (status) return status;

	return status;
}
