# Minimal Changes Required

## Summary
- 2 new files
- 4 tiny modifications to existing files

---

## 1. NEW: flowsensor.h
Complete new file - see flowsensor.h

## 2. NEW: flowsensor.cpp  
Complete new file - see flowsensor.cpp

---

## 3. MODIFY: defines.h

### Add at line 112 (after FLOWCOUNT_RT_WINDOW):
```cpp
/** Flow sensor source defines */
#define FLOW_SOURCE_HARDWARE  0x00  // read from hardware sensor
#define FLOW_SOURCE_API       0x01  // read from API
#define FLOW_SOURCE_FALLBACK  0x02  // try API first, fallback to hardware
```

### Change line 289 from:
```cpp
IOPT_RESERVE_7,
```
to:
```cpp
IOPT_FLOW_SOURCE,      // flow sensor source mode (hardware/api/fallback)
```

### Change line 310 from:
```cpp
SOPT_EMAIL_OPTS,
NUM_SOPTS // total number of string options
```
to:
```cpp
SOPT_EMAIL_OPTS,
SOPT_FLOWAPI_URL,  // flow API URL
NUM_SOPTS // total number of string options
```

---

## 4. MODIFY: main.cpp

### Add at line 29 (with other includes):
```cpp
#include "flowsensor.h"
```

### Add at line 117 (start of flow_poll function), BEFORE the hardware reading code:
```cpp
// Check flow source mode
unsigned char flow_source = os.iopts[IOPT_FLOW_SOURCE];
bool use_api = (flow_source == FLOW_SOURCE_API);
bool use_fallback = (flow_source == FLOW_SOURCE_FALLBACK);

// For API or fallback mode, check if API data is available
if(use_api || use_fallback) {
	// If API data is recent and valid, use it
	time_os_t now = os.now_tz();
	if(flowapi_success_lasttime > 0 && 
	   (now - flowapi_success_lasttime) < 30 &&
	   flowapi_errCode == FLOW_API_SUCCESS) {
		// Inject API rate into flow_last_gpm
		flow_last_gpm = flowapi_rate;
		return;
	}
	// For pure API mode, don't fallback to hardware
	if(use_api) {
		flow_last_gpm = 0;
		return;
	}
	// For fallback mode, continue to hardware reading below
}
```

### Add at line 560 (with other function declarations):
```cpp
void check_flowdata();
```

### Add at line 1195 (after check_weather()):
```cpp
// check flow data from API if enabled
check_flowdata();
```

### Add at line 1270 (after check_weather function):
```cpp
/** Check flow data from API */
void check_flowdata() {
	// Only check if flow sensor is enabled and using API or fallback mode
	if(os.iopts[IOPT_SENSOR1_TYPE] != SENSOR_TYPE_FLOW) return;
	
	unsigned char flow_source = os.iopts[IOPT_FLOW_SOURCE];
	if(flow_source != FLOW_SOURCE_API && flow_source != FLOW_SOURCE_FALLBACK) return;
	
	// do not check if network check has failed or in remote extension mode
	if (os.status.network_fails>0 || os.iopts[IOPT_REMOTE_EXT_MODE]) return;
	
	if (!os.network_connected()) return;
	
	time_os_t ntz = os.now_tz();
	// Check flow API every CHECK_FLOWAPI_INTERVAL seconds
	if (!flowapi_lasttime || (ntz > flowapi_lasttime + CHECK_FLOWAPI_INTERVAL)) {
		flowapi_lasttime = ntz;
		GetFlowData();
	}
}
```

---

## 5. MODIFY: OpenSprinkler.cpp

### Change line 429 from:
```cpp
DEFAULT_TARGET_PD_VOLTAGE,  // target pd voltage (in unit of 100mV)
0,  // reserved 7
0,  // reserved 8
```
to:
```cpp
DEFAULT_TARGET_PD_VOLTAGE,  // target pd voltage (in unit of 100mV)
FLOW_SOURCE_HARDWARE,  // flow source mode: 0=hardware, 1=api, 2=fallback
0,  // reserved 8
```

### Change line 343 from:
```cpp
210,
255,
255,
```
to:
```cpp
210,
2,  // flow source: 0=hardware, 1=api, 2=fallback
255,
```

### Change line 449 from:
```cpp
DEFAULT_EMPTY_STRING, // SOPT_EMAIL_OPTS
};
```
to:
```cpp
DEFAULT_EMPTY_STRING, // SOPT_EMAIL_OPTS
DEFAULT_EMPTY_STRING, // SOPT_FLOWAPI_URL
};
```

---

## 6. MODIFY: opensprinkler_server.cpp

### Add at line 70 (with other extern declarations):
```cpp
extern float flowapi_rate;
extern int flowapi_errCode;
```

### Change line 1304 from:
```cpp
if(os.iopts[IOPT_SENSOR1_TYPE]==SENSOR_TYPE_FLOW) {
	bfill.emit_p(PSTR("\"flcrt\":$L,\"flwrt\":$D,\"flcto\":$L,"), os.flowcount_rt, FLOWCOUNT_RT_WINDOW, flow_count);
}
```
to:
```cpp
if(os.iopts[IOPT_SENSOR1_TYPE]==SENSOR_TYPE_FLOW) {
	bfill.emit_p(PSTR("\"flcrt\":$L,\"flwrt\":$D,\"flcto\":$L,\"flsrc\":$D,\"flerr\":$D,"), 
		os.flowcount_rt, FLOWCOUNT_RT_WINDOW, flow_count, 
		os.iopts[IOPT_FLOW_SOURCE], flowapi_errCode);
}
```

---

## That's It!

Total changes:
- 2 new files (flowsensor.h, flowsensor.cpp)
- 3 lines in defines.h
- ~30 lines in main.cpp
- 3 lines in OpenSprinkler.cpp  
- 3 lines in opensprinkler_server.cpp

Everything else stays the same!


