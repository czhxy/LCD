#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <stdint.h>

typedef struct
{
    uint16_t width;
    uint16_t height;
    const uint8_t *data;
} image_t;
extern const image_t img_meihua;
extern const image_t img_dashboard;
#endif /* __IMAGE_H__ */
