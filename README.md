# LPC55S69 Online Update Application

NXP LPC55S69 application-side online firmware update framework.

The application receives an image into a staging slot, validates its metadata and CRC-32, writes a boot request, and resets. A separate bootloader must authenticate and activate the staged image. The application never overwrites the running image.

## Layout

- `include/ota/`: portable updater API and image format
- `source/ota/`: updater state machine and CRC-32
- `source/platform/`: LPC55S69 integration port (safe placeholder callbacks)
- `source/main.c`: super-loop integration example
- `docs/architecture.md`: flash layout, lifecycle, safety and integration
- `tests/`: host unit tests

## Important

This repository is an integration-ready framework, not a production bootloader. Before deployment, implement the platform callbacks with the MCUXpresso SDK, add signed-image verification, protect boot metadata, configure the linker for the application slot, and test power-loss recovery on target hardware.

## Host verification

```sh
cmake -S . -B build -DOTA_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

See [`docs/architecture.md`](docs/architecture.md) for the LPC55S69 integration procedure.
