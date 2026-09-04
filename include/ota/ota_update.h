#ifndef OTA_UPDATE_H
#define OTA_UPDATE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "ota/ota_image.h"

typedef enum
{
    OTA_STATE_IDLE = 0,
    OTA_STATE_RECEIVING,
    OTA_STATE_READY,
    OTA_STATE_FAILED
} ota_state_t;

typedef enum
{
    OTA_OK = 0,
    OTA_E_ARGUMENT,
    OTA_E_STATE,
    OTA_E_HEADER,
    OTA_E_RANGE,
    OTA_E_SEQUENCE,
    OTA_E_STORAGE,
    OTA_E_INTEGRITY,
    OTA_E_ROLLBACK,
    OTA_E_BOOT_REQUEST
} ota_result_t;

typedef struct
{
    uint32_t staging_address;
    uint32_t staging_capacity;
    uint32_t target_id;
    uint32_t current_version;
    uint32_t erase_size;
    uint32_t program_size;
} ota_config_t;

typedef struct
{
    bool (*erase)(uint32_t address, size_t length);
    bool (*program)(uint32_t address, const uint8_t *data, size_t length);
    bool (*read)(uint32_t address, uint8_t *data, size_t length);
    bool (*write_boot_request)(uint32_t image_address,
                               const ota_image_header_t *header);
} ota_platform_t;

typedef struct
{
    ota_config_t config;
    ota_platform_t platform;
    ota_image_header_t header;
    ota_state_t state;
    ota_result_t last_error;
    uint32_t received;
} ota_context_t;

ota_result_t ota_init(ota_context_t *ctx,
                      const ota_config_t *config,
                      const ota_platform_t *platform);
ota_result_t ota_begin(ota_context_t *ctx, const ota_image_header_t *header);
ota_result_t ota_write(ota_context_t *ctx, uint32_t offset,
                       const uint8_t *data, size_t length);
ota_result_t ota_finish(ota_context_t *ctx);
void ota_abort(ota_context_t *ctx);
uint32_t ota_crc32_update(uint32_t crc, const uint8_t *data, size_t length);

#endif
