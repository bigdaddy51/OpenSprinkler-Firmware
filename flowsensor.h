/* OpenSprinkler Unified Firmware
 * Copyright (C) 2015 by Ray Wang (ray@opensprinkler.com)
 *
 * Flow sensor API functions
 * Dec 2025 @ OpenSprinkler.com
 *
 * This file is part of the OpenSprinkler library
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef _FLOWSENSOR_H
#define _FLOWSENSOR_H

#include "OpenSprinkler.h"
#include "types.h"

// Flow API error codes
#define FLOW_API_SUCCESS           0
#define FLOW_API_NOT_RECEIVED     -1
#define FLOW_API_CONNECT_FAILED   -2
#define FLOW_API_TIMEOUT          -3
#define FLOW_API_PARSE_ERROR      -4

// Check flow API interval (default: every 10 seconds)
#define CHECK_FLOWAPI_INTERVAL    10

// Flow API data structure
extern int flowapi_errCode;
extern time_os_t flowapi_lasttime;
extern time_os_t flowapi_success_lasttime;
extern float flowapi_rate;         // flow rate in L/min or GPM

// Function declarations
void GetFlowData();

#endif  // _FLOWSENSOR_H

