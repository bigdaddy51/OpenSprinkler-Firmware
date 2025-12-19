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

#include <stdlib.h>
#include "OpenSprinkler.h"
#include "utils.h"
#include "opensprinkler_server.h"
#include "flowsensor.h"
#include "main.h"
#include "types.h"
#include "ArduinoJson.hpp"

extern OpenSprinkler os;
extern char tmp_buffer[];
extern char ether_buffer[];
extern const char *user_agent_string;

// Flow API state variables
int flowapi_errCode = FLOW_API_NOT_RECEIVED;
time_os_t flowapi_lasttime = 0;
time_os_t flowapi_success_lasttime = 0;
float flowapi_rate = 0;      // Real rate in L/min or GPM

unsigned char findKeyVal (const char *str,char *strbuf, uint16_t maxlen,const char *key,bool key_in_pgm=false,uint8_t *keyfound=NULL);

/**
 * Callback function to parse TenderTracker API response
 * Expected format: {"rainbird_flowrate": 1.5}
 */
static void getflowdata_callback(char* buffer) {
	char *p = buffer;
	
	// Parse JSON from TenderTracker
	if(*p == '{') {
		ArduinoJson::JsonDocument doc;
		ArduinoJson::DeserializationError error = ArduinoJson::deserializeJson(doc, p);
		
		if (!error && doc.containsKey("rainbird_flowrate")) {
			flowapi_errCode = FLOW_API_SUCCESS;
			flowapi_success_lasttime = os.now_tz();
			flowapi_rate = doc["rainbird_flowrate"];
		} else {
			flowapi_errCode = FLOW_API_PARSE_ERROR;
		}
	} else {
		flowapi_errCode = FLOW_API_PARSE_ERROR;
	}
}

/** Wrapper to peel off http header */
static void getflowdata_callback_with_peel_header(char* buffer)
{
	peel_http_header(buffer);
	getflowdata_callback(buffer);
}

/**
 * Fetch flow sensor data from external API
 * The API should return flow count and rate data
 */
void GetFlowData() {
	if(!os.network_connected()) return;
	
	// Check if flow API URL is configured
	char flowapi_url[MAX_SOPTS_SIZE];
	os.sopt_load(SOPT_FLOWAPI_URL, flowapi_url);
	if(strlen(flowapi_url) == 0) {
		flowapi_errCode = FLOW_API_NOT_RECEIVED;
		return;
	}
	
	// Build GET request
	BufferFiller bf = BufferFiller(tmp_buffer, TMP_BUFFER_SIZE*2);
	
	// Simple GET request (API can customize parameters as needed)
	bf.emit_p(PSTR("?fwv=$D"), (int)os.iopts[IOPT_FW_VERSION]);
	
	urlEncode(tmp_buffer);
	
	strcpy(ether_buffer, "GET /");
	strcat(ether_buffer, tmp_buffer);
	
	// Load flow API URL
	char *host = tmp_buffer;
	os.sopt_load(SOPT_FLOWAPI_URL, host);
	
	// Parse protocol and extract host/port
	char *host_start = host;
	
#if defined(OS_AVR)
	if (strncmp_P(host, PSTR("http://"), 7) == 0) {
		host_start = host + 7;
	} else if (strncmp_P(host, PSTR("https://"), 8) == 0) {
		host_start = host + 8;
	}
#else
	bool use_ssl = true;  // default to https
	int port = 443;       // default to https port
	
	// Check for http:// or https://
	if (strncmp_P(host, PSTR("http://"), 7) == 0) {
		use_ssl = false;
		port = 80;
		host_start = host + 7;
	} else if (strncmp_P(host, PSTR("https://"), 8) == 0) {
		use_ssl = true;
		port = 443;
		host_start = host + 8;
	}
	
	// Check for explicit port number
	char *colon = strchr(host_start, ':');
	if (colon) {
		*colon = '\0';  // null-terminate hostname
		port = atoi(colon + 1);
	}
#endif
	
	strcat(ether_buffer, " HTTP/1.0\r\nHOST: ");
	strcat(ether_buffer, host_start);
	strcat(ether_buffer, "\r\nUser-Agent: ");
	strcat(ether_buffer, user_agent_string);
	strcat(ether_buffer, "\r\n\r\n");
	
	flowapi_errCode = FLOW_API_NOT_RECEIVED;
	
#if defined(OS_AVR)
	int ret = os.send_http_request(host_start, ether_buffer, getflowdata_callback_with_peel_header);
#else
	int ret = os.send_http_request(host_start, port, ether_buffer, getflowdata_callback_with_peel_header, use_ssl);
#endif
	
	if(ret != HTTP_RQT_SUCCESS) {
		if(flowapi_errCode < 0) flowapi_errCode = ret;
	}
}

