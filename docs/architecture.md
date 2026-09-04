# LPC55S69 online-update architecture

## Scope

This project is the application-side updater. It deliberately separates transport, storage, image validation and boot activation. The running application writes only to a staging region. A small independent bootloader remains responsible for signature verification, vector-table checks, activation, trial boot, confirmation and rollback.

## Proposed lifecycle

1. Host sends `BEGIN` with the fixed image header.
2. Application checks target ID, size and monotonic firmware version.
3. Staging sectors are erased.
4. Host sends strictly sequential, program-aligned `DATA` blocks.
5. Application reads the entire staged image and verifies CRC-32.
6. Application writes a CRC-protected boot request and acknowledges the host.
7. Application resets.
8. Bootloader authenticates the signed image and trial-boots it.
9. New firmware runs self-tests and writes a confirmation marker.
10. Bootloader rolls back if the trial image is not confirmed within the policy.

CRC detects transfer corruption; it is not security. Production firmware must use a digital signature rooted in an immutable/protected public key and an anti-rollback counter. Do not accept firmware based only on CRC.

## Flash plan

The constants in `lpc55s69_ota_port.h` are placeholders, not a final memory map. Derive the final layout from the exact LPC55S69 part density, bootloader size, MCUXpresso linker map, TrustZone/CMPA/CFPA configuration, erase geometry, and the maximum application image. Reserve non-overlapping regions for bootloader, active application, staging image and redundant boot metadata.

Add linker `ASSERT` checks so neither bootloader nor application can cross a slot boundary. The bootloader and application must share one generated flash-layout header.

## MCUXpresso port checklist

- Import/create the LPCXpresso55S69 SDK project and add `ota_core` sources.
- Replace `port_erase` and `port_program` with SDK-supported flash/IAP calls.
- Honor erase/program alignment and execute required routines from RAM.
- Protect flash operations with an explicit interrupt/cache/watchdog policy.
- Implement two-copy, sequence-numbered, CRC-protected boot metadata.
- Implement reset with `NVIC_SystemReset()` after the final protocol response.
- Configure the application linker origin to the active application slot.
- Add signed manifest verification in the bootloader; consider LPC55S69 secure-boot and TrustZone features.
- Test reset/power removal at every erase, program, metadata and activation boundary.

## Transport contract

Transport is intentionally outside `ota_core`. A Modbus/UART adapter should expose `BEGIN`, `DATA`, `END`, `ABORT` and `STATUS`. Each `DATA` request includes offset, length, payload and transport-level integrity checking. Only one update session may be active. The sender retries the same offset after a lost response; the adapter must return current `received` without writing duplicate data blindly.

## Definition of done for production

- Bootloader and application build against the same flash map and image format.
- Firmware is signed and verified before execution.
- Rollback and confirmation work after watchdog resets and power loss.
- Update cannot overwrite the executing image or protected configuration.
- Boundary, malformed-header, wrong-target, downgrade, CRC, interruption and random-chunk tests pass.
- Target test confirms vector table, stack pointer, reset handler and image range before jump.
