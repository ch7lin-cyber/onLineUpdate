#ifndef LPC55S69_OTA_PORT_H
#define LPC55S69_OTA_PORT_H

#include "ota/ota_update.h"

/* Replace these values with addresses from the final linker/flash plan. */
#define LPC55S69_OTA_TARGET_ID         UINT32_C(0x55A69001)
#define LPC55S69_STAGING_ADDRESS       UINT32_C(0x00050000)
#define LPC55S69_STAGING_CAPACITY      UINT32_C(0x00028000)
#define LPC55S69_FLASH_ERASE_SIZE      UINT32_C(512)
#define LPC55S69_FLASH_PROGRAM_SIZE    UINT32_C(16)

ota_platform_t lpc55s69_ota_platform(void);
void lpc55s69_ota_system_reset(void);

#endif
