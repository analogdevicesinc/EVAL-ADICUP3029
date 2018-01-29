
#ifndef ARROW_TELEMETRY_H_
#define ARROW_TELEMETRY_H_

#if defined(__cplusplus)
extern "C" {
#endif

#define TELEMETRY_TEMPERATURE         "temperature"
#define TELEMETRY_HUMIDITY            "humidity"
#define TELEMETRY_PH                  "ph"
#define TELEMETRY_RED_LIGHT           "red_intensity"
#define TELEMETRY_GREEN_LIGHT         "green_intensity"
#define TELEMETRY_BLUE_LIGHT          "blue_intensity"
#define TELEMETRY_RED_CONCENTRATION   "red_concentration"
#define TELEMETRY_GREEN_CONCENTRATION "green_concentration"
#define TELEMETRY_BLUE_CONCENTRATION  "blue_concentration"
    
char *telemetry_serialize(void *data);

#if defined(__cplusplus)
}
#endif

#endif /* ARROW_TELEMETRY_H_ */
