#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dynamixel_sdk.h"                                   // Uses Dynamixel SDK library

// Protocol version
#define PROTOCOL_VERSION1               1.0                 // See which protocol version is used in the Dynamixel
#define PROTOCOL_VERSION2               2.0

// Default setting
#define DEVICENAME                      "COM6"      // Check which port is being used on your controller

void writeNByteTxRx(int port_num, int protocol_version, uint8_t id, uint16_t addr, uint16_t length, uint32_t value)
{
	uint8_t dxl_error = 0;
	int dxl_comm_result = COMM_TX_FAIL;

	if (length == 1)
	{
		write1ByteTxRx(port_num, protocol_version, id, addr, (uint8_t)value);
	}
	else if (length == 2)
	{
		write2ByteTxRx(port_num, protocol_version, id, addr, (uint16_t)value);
	}
	else if (length == 4)
	{
		write4ByteTxRx(port_num, protocol_version, id, addr, (uint32_t)value);
	}
	if ((dxl_comm_result = getLastTxRxResult(port_num, protocol_version)) == COMM_SUCCESS)
	{
		if ((dxl_error = getLastRxPacketError(port_num, protocol_version)) != 0)
			printf("%s\n", getRxPacketError(protocol_version, dxl_error));
		//fprintf(stderr, "\n Success to write\n\n");
	}
	else
	{
		printf("%s\n", getTxRxResult(protocol_version, dxl_comm_result));
		fprintf(stderr, "\n Fail to write! \n\n");
	}
}

int readNByteTxRx(int port_num, int protocol_version, uint8_t id, uint16_t addr, uint16_t length)
{
	uint8_t dxl_error = 0;
	int     dxl_comm_result = COMM_TX_FAIL;

	int8_t  value8 = 0;
	int16_t value16 = 0;
	int32_t value32 = 0;

	if (length == 1)
	{
		value8 = read1ByteTxRx(port_num, protocol_version, id, addr);
	}
	else if (length == 2)
	{
		value16 = read2ByteTxRx(port_num, protocol_version, id, addr);
	}
	else if (length == 4)
	{
		value32 = read4ByteTxRx(port_num, protocol_version, id, addr);
	}

	if ((dxl_comm_result = getLastTxRxResult(port_num, protocol_version)) == COMM_SUCCESS)
	{
		if ((dxl_error = getLastRxPacketError(port_num, protocol_version)) != 0)
			printf("%s\n", getRxPacketError(protocol_version, dxl_error));

		if (length == 1)
		{
			fprintf(stderr, "\n READ VALUE : (UNSIGNED) %u , (SIGNED) %d \n\n", (uint8_t)value8, value8);
			return value8;
		}
		else if (length == 2)
		{
			fprintf(stderr, "\n READ VALUE : (UNSIGNED) %u , (SIGNED) %d \n\n", (uint16_t)value16, value16);
			return value16;
		}
		else if (length == 4)
		{
			fprintf(stderr, "\n READ VALUE : (UNSIGNED) %u , (SIGNED) %d \n\n", (uint32_t)value32, value32);
			return value32;
		}
	}

	else
	{

		printf("%s\n", getTxRxResult(protocol_version, dxl_comm_result));
		fprintf(stderr, "\n Fail to write! \n\n");
	}
}

int main(int argc, char *argv[])
{

	char *dev_name = (char*)DEVICENAME;


	// Initialize PortHandler Structs
	// Set the port path
	// Get methods and members of PortHandlerLinux or PortHandlerWindows
	int port_num = portHandler(dev_name);

	// Initialize PacketHandler Structs
	packetHandler();
	setBaudRate(port_num, 3000000);
	// Open port
	if (openPort(port_num))
	{
		printf("Succeeded to open the port!\n\n");
		printf(" - Device Name : %s\n", dev_name);
		printf(" - Baudrate    : %d\n\n", getBaudRate(port_num));
	}
	else
	{
		printf("Failed to open the port! [%s]\n", dev_name);
		printf("Press any key to terminate...\n");
		getch();
		return 0;
	}

	char    input[128];
	char    cmd[80];
	char    param[20][30];
	int     num_param;
	char*   token;
	uint8_t dxl_error;

	int pos_1;
	int pos_2;
	int pos_3;
	int pos_4;

	int motor_id[4];
	motor_id[0] = 53;
	motor_id[1] = 54;
	motor_id[2] = 55;
	motor_id[3] = 56;

	int initial1 = 3533;
	int initial2 = 1479;
	int initial3 = 1721;
	int initial4 = 433;
	
	printf("Torque ON! \n");

	writeNByteTxRx(port_num, PROTOCOL_VERSION2, motor_id[0], 64, 1, 1);
	writeNByteTxRx(port_num, PROTOCOL_VERSION2, motor_id[1], 64, 1, 1);
	writeNByteTxRx(port_num, PROTOCOL_VERSION2, motor_id[2], 64, 1, 1);
	writeNByteTxRx(port_num, PROTOCOL_VERSION2, motor_id[3], 64, 1, 1);

	while (1)
	{
		int target_angle1;
		int target_angle2;
		int target_angle3;
		int target_angle4;

		int target_pos1;
		int target_pos2;
		int target_pos3;
		int target_pos4;

		double angle1;
		double angle2;
		double angle3;
		double angle4;

		printf("Target Position1: ");
		scanf("%lf", &angle1);

		if (angle1==999)
		{
			break;
		}

		printf("Target Position2: ");
		scanf("%lf", &angle2);

		printf("Target Position3: ");
		scanf("%lf", &angle3);

		printf("Target Position4: ");
		scanf("%lf", &angle4);


		pos_1 = readNByteTxRx(port_num, PROTOCOL_VERSION2, motor_id[0], 132, 4);
		pos_2 = readNByteTxRx(port_num, PROTOCOL_VERSION2, motor_id[1], 132, 4);
		pos_3 = readNByteTxRx(port_num, PROTOCOL_VERSION2, motor_id[2], 132, 4);
		pos_4 = readNByteTxRx(port_num, PROTOCOL_VERSION2, motor_id[3], 132, 4);

		printf("pos_1 : %d\n", pos_1);
		printf("pos_2 : %d\n", pos_2);
		printf("pos_3 : %d\n", pos_1);
		printf("pos_4 : %d\n", pos_2);

		target_angle1 = (angle1) * 2048. / 180.; //-179~+180;
		target_pos1 = initial1 - target_angle1;

		target_angle2 = (angle2) * 2048. / 180.;
		target_pos2 = initial2 - target_angle2;

		target_angle3 = (angle3) * 2048. / 180.;
		target_pos3 = initial3 - target_angle3;

		target_angle4 = (angle4) * 2048. / 180.;
		target_pos4 = initial4 + target_angle4;

		
		if ((target_pos1 < 0)||(angle1<-20))
		{
			printf("Unavilable joint1 input, Shut down \n");
			printf("target_pos1 : %d\n\n", target_pos1);
			break;
		}

		if ((target_pos2 < 0) || (target_pos2 < 800) || (target_pos2 > 1620))
		{
			printf("Unavilable joint2 input, Shut down \n");
			printf("target_pos2 : %d\n\n", target_pos2);
			break;
		}

		if ((target_pos3 < 0) || (target_pos3 < initial3-800) || (target_pos3 > initial3+20))
		{
			printf("Unavilable joint3 input, Shut down \n");
			printf("target_pos3 : %d\n\n", target_pos3);
			break;
		}

		if ((target_pos4 < 0) || (target_pos4 < initial4 - 800) || (target_pos4 > initial4 + 20))
		{
			printf("Unavilable joint4 input, Shut down \n");
			printf("target_pos4 : %d\n\n", target_pos4);
			break;
		}



		writeNByteTxRx(port_num, PROTOCOL_VERSION2, motor_id[0], 116, 4, target_pos1);
		writeNByteTxRx(port_num, PROTOCOL_VERSION2, motor_id[1], 116, 4, target_pos2);
		writeNByteTxRx(port_num, PROTOCOL_VERSION2, motor_id[2], 116, 4, target_pos3);
		writeNByteTxRx(port_num, PROTOCOL_VERSION2, motor_id[3], 116, 4, target_pos4);

	}

	writeNByteTxRx(port_num, PROTOCOL_VERSION2, motor_id[0], 64, 1, 0);
	writeNByteTxRx(port_num, PROTOCOL_VERSION2, motor_id[1], 64, 1, 0);
	writeNByteTxRx(port_num, PROTOCOL_VERSION2, motor_id[2], 64, 1, 0);
	writeNByteTxRx(port_num, PROTOCOL_VERSION2, motor_id[3], 64, 1, 0);
	printf("Torque OFF! \n");
	printf("We are done. Waiting for exit... \n");

	system("pause");
}