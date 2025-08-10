# ESPHome External Component: VL53L1X (ToF)

A lightweight ESPHome external component for the **VL53L1X** time‑of‑flight distance sensor (e.g., TOF400C modules). It exposes tunable **distance mode**, **timing budget** and **inter‑measurement period** directly in YAML. Designed to feel like a "digital ruler" with optional on‑device smoothing via ESPHome filters.

> Works on ESP32 (Arduino framework) and ESP8266. Uses the Pololu VL53L1X driver internally.

---

## Features

- Configurable **distance modes**: `short`, `medium`, `long` (up to \~4 m with *long*).
- Adjustable **timing budget** (integration time) for stability vs. speed.
- Adjustable **inter‑measurement** period for clean, periodic sampling.
- **I²C address** selectable in YAML (default `0x29`).
- Plays nicely with ESPHome’s filters (median, sliding window, delta, throttle, etc.).

---

## Requirements

- **ESPHome** (recent version).
- **ESP32** (Arduino framework) or **ESP8266**.
- I²C wiring with proper pull‑ups (SDA/SCL), typical `400 kHz` bus.

> If you’re on ESP32, make sure you build with the **Arduino** framework:
>
> ```yaml
> esp32:
>   board: esp32-s3-devkitc-1   # or your board
>   framework:
>     type: arduino
> ```

---

## Installation

You can install the component **locally** or as a **repo**. Two layouts are supported—pick **one**.

### Option A — Local components root (simple)

```
/config/
├─ your-device.yaml
└─ my_components/
   └─ tof_vl53l1x/
      ├─ __init__.py
      ├─ sensor.py
      ├─ tof_vl53l1x.h
      ├─ VL53L1X.cpp   # Pololu driver (local copy) – optional if using registry lib
      └─ VL53L1X.h
```

YAML:

```yaml
aexternal_components:
  - source: /config/my_components
    refresh: always
```

### Option B — Repo-style layout

```
/config/
├─ your-device.yaml
└─ my_components/
   └─ components/
      └─ tof_vl53l1x/
         ├─ __init__.py
         ├─ sensor.py
         ├─ tof_vl53l1x.h
         ├─ VL53L1X.cpp
         └─ VL53L1X.h
```

YAML:

```yaml
external_components:
  - source: /config/my_components/components
    refresh: always
```

> **Pololu driver source**: Either keep `VL53L1X.cpp/.h` locally (as above) **or** remove them and add the Arduino registry library instead:
>
> ```yaml
> esphome:
>   libraries:
>     - "pololu/VL53L1X"
> ```
>
> Use **only one** of these approaches to avoid duplicate class definitions.

---

## Wiring & I²C

Minimal example (ESP32):

```yaml
i2c:
  sda: GPIO8
  scl: GPIO9
  scan: true
  frequency: 400kHz
```

---

## YAML Usage

```yaml
sensor:
  - platform: tof_vl53l1x
    name: "Distance"
    distance_mode: long        # short | medium | long
    timing_budget_ms: 150000   # integration time (e.g., 50_000..200_000)
    intermeasurement_ms: 250   # measurement period; will be clamped to >= timing budget
    update_interval: 250ms     # ESPHome polling; keep >= intermeasurement_ms
    filters:
      - sliding_window_moving_average:
          window_size: 3
          send_every: 1
    accuracy_decimals: 0
    unit_of_measurement: "mm"
```

### Parameters

- **distance\_mode** *(enum)*: `short`, `medium`, or `long`.
  - *short*: best stability/ambient immunity, \~1–1.3 m range.
  - *medium*: balanced.
  - *long*: maximum range (\~4 m), more jitter.
- **timing\_budget\_ms** *(uint32)*: Integration time per measurement.
  - Larger values lead to **less noise** and more stable readings.
  - Typical: `50_000`, `100_000`, `150_000`, `200_000`.
- **intermeasurement\_ms** *(uint32)*: Period between measurements in continuous mode.
  - Effective sample rate ≈ `1000 / intermeasurement_ms` Hz.
  - Should be **≥ timing\_budget\_ms** (the component will enforce this).
- **update\_interval** *(duration)*: How often ESPHome asks the component to publish.
  - Keep **≥ intermeasurement\_ms** so you don’t request data faster than it is produced.

---

## Tuning Guide

- **Smoother readings**: increase `timing_budget_ms` (e.g., 100–200 ms) and keep `intermeasurement_ms` ≥ that; add a `median` filter or increase `sliding_window_moving_average.window_size`.
- **Faster response**: reduce both `timing_budget_ms` and `intermeasurement_ms` together (e.g., 50–100 ms). Consider lower filter strength.
- **Digital ruler feel**: `distance_mode: long`, `timing_budget_ms: 100_000–150_000`, `intermeasurement_ms: same or +50 ms`, `window_size: 3`.

> Typical jitter of **±5–10 mm** is normal for VL53L1X, especially in `long` mode and at low target reflectance.

---

## Multi‑sensor (XSHUT) Setup

VL53L1X boots at `0x29`. To run multiple sensors on the same bus, you must control their **XSHUT** pins so only one sensor is active while assigning a new address:

1. Hold all sensors in reset (XSHUT low).
2. Enable one sensor (XSHUT high), wait \~2–3 ms.
3. Initialize it (default `0x29`) and call `setAddress(new_addr)` via this component.
4. Repeat for the next sensor with a different address.

> This component sets the target address **after** a successful `init()` at `0x29`. Use separate ESPHome GPIO outputs to control XSHUT lines and sequence sensors if you need more than one.

---

## Troubleshooting

- \*\*Platform not found: \*\*\`\`

  - Check folder layout (see *Installation*). Ensure `__init__.py` exists.
  - `external_components.source` must point to the parent folder of `tof_vl53l1x/` (or of `components/`).
  - Use `refresh: always` and perform a **Clean build** in ESPHome.

- `** / **`\*\* not found\*\*

  - Build ESP32 with `framework: { type: arduino }`.
  - If you use the Arduino registry library *and* local `VL53L1X.cpp/.h`, remove one to avoid duplicates.

- \*\*Redefinition of \*\*\`\`

  - You included **both** local `VL53L1X.cpp/.h` and the Arduino registry lib. Use **only one**.

- \*\*Timeout during \*\*\`\`

  - Ensure `intermeasurement_ms >= timing_budget_ms`.
  - Set `update_interval >= intermeasurement_ms`.
  - The component also checks `dataReady()` before reading to avoid blocking.

- **No device at 0x29**

  - Check wiring and pull‑ups; try `frequency: 100kHz` temporarily.
  - Verify the module’s default address and that only one device is active at a time when changing addresses.

---

## Development Notes

- The component wraps the Pololu driver and integrates with ESPHome’s `PollingComponent` and `sensor::Sensor` APIs.
- I²C is initialized by ESPHome; the component does **not** call `Wire.begin()`.
- Address change is applied **after** `init()` at default `0x29`.

---

## License

MIT. See `LICENSE`.

---

## Credits

- [Pololu VL53L1X Arduino library]
- STMicroelectronics VL53L1X sensor.
- ESPHome community.

---

### Example: Minimal YAML

```yaml
external_components:
  - source: /config/my_components
    refresh: always

i2c:
  sda: GPIO8
  scl: GPIO9
  scan: true
  frequency: 400kHz

esp32:
  board: esp32-s3-devkitc-1
  framework:
    type: arduino

sensor:
  - platform: tof_vl53l1x
    name: "Distance"
    distance_mode: long
    timing_budget_ms: 150000
    intermeasurement_ms: 250
    update_interval: 250ms
    filters:
      - sliding_window_moving_average:
          window_size: 3
          send_every: 1
    accuracy_decimals: 0
    unit_of_measurement: "mm"
```

