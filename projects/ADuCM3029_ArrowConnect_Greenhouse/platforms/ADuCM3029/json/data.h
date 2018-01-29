#if !defined(_IOT_DATA_H_)
#define _IOT_DATA_H_

typedef struct {
  float    temperature;
  float    humidity;
  float    ph;
  float    red_intensity;
  float    green_intensity;
  float    blue_intensity;
  float    red_concentration;
  float    green_concentration;
  float    blue_concentration;

} sensors_data_t;

#endif
