# Architecture

CacheWraith is split into small components with one main responsibility each.

## Main Flow

```text
main.cpp
  loads Config
  initializes Logger
  creates Application

Application
  registers monitors
  runs monitor checks every interval
  passes alerts to AlertManager

AlertManager
  applies cooldowns
  sends alerts through TelegramNotifier
  logs local alert activity
```

## Monitor Interface

All monitors implement:

```cpp
class IMonitor {
public:
    virtual ~IMonitor() = default;
    virtual std::vector<Alert> check() = 0;
    virtual std::string name() const = 0;
};
```

This keeps monitors easy to add, remove, and test.

## Health Monitors

- `RamMonitor`: reads `/proc/meminfo`
- `DiskMonitor`: uses `statvfs`
- `CpuMonitor`: reads `/proc/loadavg`
- `ServiceMonitor`: calls `systemctl is-active --quiet` without a shell
- `WebsiteMonitor`: uses libcurl with timeouts

## Security Monitors

- `SshLogMonitor`: parses SSH auth logs
- `NginxLogMonitor`: parses Nginx access logs
- `LogTailer`: reads only new lines during runtime

The parsing functions are separate from monitor classes so tests can call them directly.

## Alerts

An alert contains:

- `type`: machine-friendly alert type
- `key`: target for cooldown identity
- `title`: human alert title
- `message`: alert body
- `severity`: info, warning, or critical
- `bypass_cooldown`: used for successful SSH login alerts

## Cooldowns

Cooldown identity uses:

```text
alert_type:alert_key
```

Examples:

```text
service_down:nginx
website_down:https://example.com
ssh_bruteforce:1.2.3.4
```

## Project Layout

```text
include/cachewraith/  public headers
src/                  implementation
tests/                simple parser and cooldown tests
systemd/              service file example
docs/                 project documentation
```
