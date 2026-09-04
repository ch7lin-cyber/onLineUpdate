#include "lpc55s69_ota_port.h"

/*
 * Integration boundary for MCUXpresso SDK.
 * Implement with the LPC55S69 flash/IAP driver while executing flash commands
 * from RAM as required by the selected SDK. Disable only the interrupts that
 * can execute from the bank being programmed. Feed the watchdog deliberately.
 */
static bool port_erase(uint32_t address, size_t length)
{
    (void)address; (void)length;
    return false;
}

static bool port_program(uint32_t address, const uint8_t *data, size_t length)
{
    (void)address; (void)data; (void)length;
    return false;
}

static bool port_read(uint32_t address, uint8_t *data, size_t length)
{
    const uint8_t *source = (const uint8_t *)(uintptr_t)address;
    size_t i;
    for (i = 0U; i < length; ++i) data[i] = source[i];
    return true;
}

static bool port_write_boot_request(uint32_t image_address,
                                    const ota_image_header_t *header)
{
    (void)image_address; (void)header;
    /* Store a CRC-protected request in a dedicated metadata erase sector. */
    return false;
}

ota_platform_t lpc55s69_ota_platform(void)
{
    const ota_platform_t platform = {
        port_erase, port_program, port_read, port_write_boot_request
    };
    return platform;
}

void lpc55s69_ota_system_reset(void)
{
    /* Call NVIC_SystemReset() after transport response has completed. */
    for (;;) { }
}
