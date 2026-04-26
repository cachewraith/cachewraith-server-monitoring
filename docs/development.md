# Development

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
```

## Tests

```bash
ctest --test-dir build --output-on-failure
```

Current tests cover:

- SSH failed login parsing
- SSH accepted login parsing
- Nginx suspicious path parsing
- cooldown behavior

## Run Locally

Use a local log file to avoid root-only `/var/log` permissions:

```json
"log_file": "./cachewraith-monitor.log"
```

Then:

```bash
./build/cachewraith-monitor --config ./config.json --once
```

## Add a Monitor

1. Create a header in `include/cachewraith/monitor/`.
2. Create an implementation in `src/monitor/`.
3. Implement `IMonitor`.
4. Register it in `Application::registerMonitors`.
5. Add tests if parsing or alert logic is involved.

## Code Style

- Use namespace `cachewraith`.
- Keep classes small.
- Keep monitor responsibilities separate.
- Prefer standard library facilities.
- Prefer RAII.
- Use `std::chrono` for time.
- Do not hardcode secrets.
- Add comments only when they clarify non-obvious behavior.
