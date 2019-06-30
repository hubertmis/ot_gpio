/*
 * ft800_host_commands.h
 *
 *  Created on: 16 kwi 2014
 *      Author: bearh
 */

#ifndef FT8XX_HOST_COMMANDS_H_
#define FT8XX_HOST_COMMANDS_H_

typedef enum {
	ACTIVE = 0,
	STANDBY = 0x41,
	SLEEP   = 0x42,
	PWRDOWN = 0x50,

	CLKEXT  = 0x44,
	CLK48M  = 0x62,
	CLK36M  = 0x61,

	CORERST = 0x68
} ft800_command_t;

#endif /* FT8XX_HOST_COMMANDS_H_ */
