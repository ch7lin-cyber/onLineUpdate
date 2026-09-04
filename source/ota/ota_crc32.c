#include "ota/ota_update.h"

uint32_t ota_crc32_update(uint32_t crc, const uint8_t *data, size_t length)
{
    size_t i;
    uint32_t bit;

    if ((data == NULL) && (length != 0U))
    {
        return crc;
    }

    for (i = 0U; i < length; ++i)
    {
        crc ^= data[i];
        for (bit = 0U; bit < 8U; ++bit)
        {
            const uint32_t mask = 0U - (crc & 1U);
            crc = (crc >> 1U) ^ (UINT32_C(0xEDB88320) & mask);
        }
    }
    return crc;
}
