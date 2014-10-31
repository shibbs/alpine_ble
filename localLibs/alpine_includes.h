//alpine_includes.h

#ifndef ALPINE_INCLUDES_H_
#define ALPINE_INCLUDES_H_

#include <string.h>
#include "nordic_common.h"
#include "radian_stepper.h"

#define MICHRON						true

#define SHUTTER_PACKET_LEN 2

#define MAX_BLE_PACKET_LEN	20

#define TL_PACKET_PREAMBLE_LEN	4
#define TL_PACKET_STD_LEN	30
#define TL_PACKET_MAX_QUEUES 3
#define TL_PACKET_MAX_LEN (TL_PACKET_STD_LEN * TL_PACKET_MAX_QUEUES)
#define TL_SUB_PACKET_LEN		(MAX_BLE_PACKET_LEN - 1)

#define TL_PACKET_MAX_VAL			240
#define TL_PACKET_START_FLAG	241
#define TL_PACKET_END_FLAG		243

#endif //ALPINE_INCLUDES_H_
