#include "ota/ota_update.h"
#include <string.h>

static ota_result_t fail(ota_context_t *ctx, ota_result_t error)
{
    ctx->state = OTA_STATE_FAILED;
    ctx->last_error = error;
    return error;
}

ota_result_t ota_init(ota_context_t *ctx, const ota_config_t *config,
                      const ota_platform_t *platform)
{
    if ((ctx == NULL) || (config == NULL) || (platform == NULL) ||
        (platform->erase == NULL) || (platform->program == NULL) ||
        (platform->read == NULL) || (platform->write_boot_request == NULL) ||
        (config->staging_capacity == 0U) || (config->erase_size == 0U) ||
        (config->program_size == 0U))
    {
        return OTA_E_ARGUMENT;
    }

    memset(ctx, 0, sizeof(*ctx));
    ctx->config = *config;
    ctx->platform = *platform;
    ctx->state = OTA_STATE_IDLE;
    return OTA_OK;
}

ota_result_t ota_begin(ota_context_t *ctx, const ota_image_header_t *header)
{
    uint32_t erase_length;

    if ((ctx == NULL) || (header == NULL)) return OTA_E_ARGUMENT;
    if (ctx->state == OTA_STATE_RECEIVING) return OTA_E_STATE;
    if ((header->magic != OTA_IMAGE_MAGIC) ||
        (header->format_version != OTA_FORMAT_VERSION) ||
        (header->header_size != sizeof(*header)) ||
        (header->target_id != ctx->config.target_id))
        return fail(ctx, OTA_E_HEADER);
    if ((header->image_size == 0U) ||
        (header->image_size > ctx->config.staging_capacity) ||
        ((header->image_size % ctx->config.program_size) != 0U) ||
        ((ctx->config.staging_address % ctx->config.erase_size) != 0U))
        return fail(ctx, OTA_E_RANGE);
    if (header->firmware_version <= ctx->config.current_version)
        return fail(ctx, OTA_E_ROLLBACK);

    erase_length = (header->image_size / ctx->config.erase_size) *
                   ctx->config.erase_size;
    if ((header->image_size % ctx->config.erase_size) != 0U)
        erase_length += ctx->config.erase_size;
    if ((erase_length > ctx->config.staging_capacity) ||
        !ctx->platform.erase(ctx->config.staging_address, erase_length))
        return fail(ctx, OTA_E_STORAGE);

    ctx->header = *header;
    ctx->received = 0U;
    ctx->last_error = OTA_OK;
    ctx->state = OTA_STATE_RECEIVING;
    return OTA_OK;
}

ota_result_t ota_write(ota_context_t *ctx, uint32_t offset,
                       const uint8_t *data, size_t length)
{
    if ((ctx == NULL) || ((data == NULL) && (length != 0U))) return OTA_E_ARGUMENT;
    if (ctx->state != OTA_STATE_RECEIVING) return OTA_E_STATE;
    if (offset != ctx->received) return fail(ctx, OTA_E_SEQUENCE);
    if ((ctx->received > ctx->header.image_size) || (length == 0U) ||
        ((length % ctx->config.program_size) != 0U) ||
        (length > (size_t)(ctx->header.image_size - ctx->received)))
        return fail(ctx, OTA_E_RANGE);
    if (!ctx->platform.program(ctx->config.staging_address + offset, data, length))
        return fail(ctx, OTA_E_STORAGE);
    ctx->received += (uint32_t)length;
    return OTA_OK;
}

ota_result_t ota_finish(ota_context_t *ctx)
{
    uint8_t buffer[256];
    uint32_t offset = 0U;
    uint32_t crc = UINT32_C(0xFFFFFFFF);

    if (ctx == NULL) return OTA_E_ARGUMENT;
    if ((ctx->state != OTA_STATE_RECEIVING) ||
        (ctx->received != ctx->header.image_size)) return OTA_E_STATE;

    while (offset < ctx->header.image_size)
    {
        const uint32_t remaining = ctx->header.image_size - offset;
        const size_t count = remaining < sizeof(buffer) ? remaining : sizeof(buffer);
        if (!ctx->platform.read(ctx->config.staging_address + offset, buffer, count))
            return fail(ctx, OTA_E_STORAGE);
        crc = ota_crc32_update(crc, buffer, count);
        offset += (uint32_t)count;
    }
    crc ^= UINT32_C(0xFFFFFFFF);
    if (crc != ctx->header.image_crc32) return fail(ctx, OTA_E_INTEGRITY);
    if (!ctx->platform.write_boot_request(ctx->config.staging_address, &ctx->header))
        return fail(ctx, OTA_E_BOOT_REQUEST);
    ctx->state = OTA_STATE_READY;
    return OTA_OK;
}

void ota_abort(ota_context_t *ctx)
{
    if (ctx != NULL)
    {
        memset(&ctx->header, 0, sizeof(ctx->header));
        ctx->received = 0U;
        ctx->last_error = OTA_OK;
        ctx->state = OTA_STATE_IDLE;
    }
}
