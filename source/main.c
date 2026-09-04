#include "ota/ota_update.h"
#include "platform/lpc55s69_ota_port.h"

static ota_context_t g_ota;

static void application_init(void)
{
    const ota_config_t config = {
        LPC55S69_STAGING_ADDRESS,
        LPC55S69_STAGING_CAPACITY,
        LPC55S69_OTA_TARGET_ID,
        UINT32_C(0x00010000), /* replace with generated app version */
        LPC55S69_FLASH_ERASE_SIZE,
        LPC55S69_FLASH_PROGRAM_SIZE
    };
    const ota_platform_t platform = lpc55s69_ota_platform();
    (void)ota_init(&g_ota, &config, &platform);
}

static void application_process(void)
{
    /*
     * Poll the chosen transport and map commands to:
     * OTA_BEGIN -> ota_begin(), OTA_DATA -> ota_write(),
     * OTA_END -> ota_finish(), OTA_ABORT -> ota_abort().
     * Reset only after OTA_END success has been acknowledged to the sender.
     */
    if (g_ota.state == OTA_STATE_READY)
        lpc55s69_ota_system_reset();
}

int main(void)
{
    application_init();
    for (;;) application_process();
}
