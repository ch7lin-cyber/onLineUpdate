#include "ota/ota_update.h"
#include <assert.h>
#include <string.h>

static uint8_t flash_area[1024];
static bool boot_requested;

static bool erase_cb(uint32_t address, size_t length)
{ (void)address; if (length > sizeof(flash_area)) return false; memset(flash_area, 0xFF, length); return true; }
static bool program_cb(uint32_t address, const uint8_t *data, size_t length)
{ if ((address + length) > sizeof(flash_area)) return false; memcpy(&flash_area[address], data, length); return true; }
static bool read_cb(uint32_t address, uint8_t *data, size_t length)
{ if ((address + length) > sizeof(flash_area)) return false; memcpy(data, &flash_area[address], length); return true; }
static bool request_cb(uint32_t address, const ota_image_header_t *header)
{ (void)address; (void)header; boot_requested = true; return true; }

int main(void)
{
    ota_context_t ctx;
    const uint8_t image[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    uint32_t crc = ota_crc32_update(UINT32_C(0xFFFFFFFF), image, sizeof(image)) ^ UINT32_C(0xFFFFFFFF);
    const ota_config_t cfg = {0U, sizeof(flash_area), 0x55A69001U, 1U, 512U, 16U};
    const ota_platform_t platform = {erase_cb, program_cb, read_cb, request_cb};
    ota_image_header_t header = {OTA_IMAGE_MAGIC, OTA_FORMAT_VERSION,
        sizeof(ota_image_header_t), sizeof(image), crc, 2U, 0x55A69001U, 0U, {0}};

    assert(ota_init(&ctx, &cfg, &platform) == OTA_OK);
    assert(ota_begin(&ctx, &header) == OTA_OK);
    assert(ota_write(&ctx, 1U, image, sizeof(image)) == OTA_E_SEQUENCE);
    ota_abort(&ctx);
    assert(ota_begin(&ctx, &header) == OTA_OK);
    assert(ota_write(&ctx, 0U, image, sizeof(image)) == OTA_OK);
    assert(ota_finish(&ctx) == OTA_OK);
    assert(ctx.state == OTA_STATE_READY && boot_requested);

    header.firmware_version = 1U;
    ota_abort(&ctx);
    assert(ota_begin(&ctx, &header) == OTA_E_ROLLBACK);
    return 0;
}
