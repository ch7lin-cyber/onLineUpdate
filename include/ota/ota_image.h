#ifndef OTA_IMAGE_H
#define OTA_IMAGE_H

#include <stdint.h>

#define OTA_IMAGE_MAGIC       UINT32_C(0x4F544131) /* OTA1 */
#define OTA_FORMAT_VERSION    UINT16_C(1)

typedef struct
{
    uint32_t magic;
    uint16_t format_version;
    uint16_t header_size;
    uint32_t image_size;
    uint32_t image_crc32;
    uint32_t firmware_version;
    uint32_t target_id;
    uint32_t flags;
    uint32_t reserved[5];
} ota_image_header_t;

#endif
