/*
 * ov2640_jpeg.h
 *
 *  Created on: Aug 17, 2024
 *      Author: user
 */

#ifndef INC_OV2640_JPEG_H_
#define INC_OV2640_JPEG_H_

#include "main.h"

#define OV2640_I2C &hi2c4
#define OV2640_ADDR 0x60

#define OV2640_DEBUG_UART &huart3

#define OV2640_DSP_RA_DLMT              0xFF
#define OV2640_SENSOR_COM7              0x12

#define JPEG_BUFF_SIZE	(92160*3) //270k for still jpeg

uint8_t  OV2640_Mem_Write(uint8_t reg, uint8_t val);
uint16_t OV2640_Init_JPEG(void);
uint8_t OV2640_TakePicture(uint32_t* jpeg_begin, uint32_t* jpeg_end);

#endif /* INC_OV2640_JPEG_H_ */
