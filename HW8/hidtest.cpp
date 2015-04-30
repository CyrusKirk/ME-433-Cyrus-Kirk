#ifdef WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include "hidapi.h"

#define MAX_STR 255
#define MAX_MSG 100
#define DATA_PTS 200

int main(int argc, char* argv[])
{
	int res;
	unsigned char buf[65];
	wchar_t wstr[MAX_STR];
	hid_device *handle;
	int i;
	int j = 0;
	short accX[DATA_PTS];
	short accY[DATA_PTS];
	short accZ[DATA_PTS];

	// Initialize the hidapi library
	res = hid_init();

	// Open the device using the VID, PID,
	// and optionally the Serial number.
	handle = hid_open(0x4d8, 0x3f, NULL);

	// Read the Manufacturer String
	res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
	wprintf(L"Manufacturer String: %s\n", wstr);

	// Read the Product String
	res = hid_get_product_string(handle, wstr, MAX_STR);
	wprintf(L"Product String: %s\n", wstr);

	// Read the Serial Number String
	res = hid_get_serial_number_string(handle, wstr, MAX_STR);
	wprintf(L"Serial Number String: (%d) %s\n", wstr[0], wstr);

	// Read Indexed String 1
	res = hid_get_indexed_string(handle, 1, wstr, MAX_STR);
	wprintf(L"Indexed String 1: %s\n", wstr);
	
	char message[MAX_MSG];
	wprintf(L"Input message:\n");
	wscanf(L"%[^\n]s", message);
	
	int row;
	wprintf(L"Input row:\n", wstr);
	wscanf(L"%d", &row);
	
	// Toggle LED (cmd 0x80). The first byte is the report number (0x0).
	buf[0] = 0x0;
	buf[1] = 0x80;
	buf[3] = row;
	for (i = 0; i < MAX_MSG; i++)
		buf[5+i] = message[i];
	
	
	res = hid_write(handle, buf, 65);

	// Request state (cmd 0x81). The first byte is the report number (0x0).
	buf[0] = 0x0;
	buf[1] = 0x81;
	res = hid_write(handle, buf, 65);
	
	// Read requested state
	while (j < DATA_PTS)
	{
		buf[0] = 0x0;
		buf[1] = 0x81;
		res = hid_write(handle, buf, 65);
		res = hid_read(handle, buf, 65);
		if (!((buf[5] == 0) && (buf[6] == 0)))//just checking z for 0s
		{
			accX[j] = (short)((buf[1] << 8) | (buf[2]));
			accY[j] = (short)((buf[3] << 8) | (buf[4]));
			accZ[j] = (short)((buf[5] << 8) | (buf[6]));
			//wprintf(L"X acceleration: %d\nY acceleration: %d\nZ acceleration: %d\n", accX[j], accY[j], accZ[j]);
			j++;
		}
	}
	
	//Save to a file
	wprintf(L"Saving to file\n");
	FILE *ofp;
	ofp = fopen("accels.txt", "w");
	for (i=0; i<DATA_PTS; i++)
		fwprintf(ofp,L"X: %d  Y: %d  Z: %d\r\n",accX[i],accY[i],accZ[i]);
	fclose(ofp);
	
	// Finalize the hidapi library
	res = hid_exit();

	return 0;
}