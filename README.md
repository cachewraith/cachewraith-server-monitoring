# CacheWraith Server Monitoring

CacheWraith is a lightweight C++20 Linux VPS monitoring and security alert agent. It checks resource health, watches selected services and websites, tails security logs, and sends actionable alerts to Telegram.

## Features

- Telegram alerts through libcurl
- RAM, disk, and 1-minute load average monitoring
- `systemctl is-active` service checks with service-name validation
- Website HTTP health checks with timeouts
- SSH failed login, successful password login, and successful public-key login detection
- Nginx suspicious path, request rate, and 404 burst detection
- Runtime log tailing that avoids rereading large old logs
- Alert cooldowns to reduce Telegram spam
- Optional Telegram forum topic routing with `message_thread_id`
- JSON configuration with secrets kept in environment variables
- systemd service example
- Basic parser and cooldown tests

## Dependencies

Ubuntu/Debian:

```bash
sudo apt update
sudo apt install -y build-essential cmake libcurl4-openssl-dev nlohmann-json3-dev
```

Arch Linux:

```bash
sudo pacman -S --needed base-devel cmake curl nlohmann-json
```

If `nlohmann-json` is not installed, CMake will try to fetch it at configure time.

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
```

The binary is created at:

```bash
./build/cachewraith-monitor
```

## Telegram Setup

1. Open Telegram and message `@BotFather`.
2. Create a bot with `/newbot`.
3. Save the bot token securely.
4. Send a message to the bot from the target chat.
5. Get the chat ID using a trusted method such as Telegram `getUpdates`.

Set secrets as environment variables. Do not put them in `config.json`.

```bash
export CACHEWRAITH_TELEGRAM_BOT_TOKEN="123456:replace-me"
export CACHEWRAITH_TELEGRAM_CHAT_ID="123456789"
```

For systemd, create `/etc/cachewraith-monitor/cachewraith.env`:

```bash
CACHEWRAITH_TELEGRAM_BOT_TOKEN=123456:replace-me
CACHEWRAITH_TELEGRAM_CHAT_ID=123456789
```

Protect it:

```bash
sudo chmod 600 /etc/cachewraith-monitor/cachewraith.env
```

## Configuration

Start from the example:

```bash
cp config.example.json config.json
```

Important fields:

- `server_name`: shown in every alert
- `interval_seconds`: monitoring loop interval
- `log_file`: CacheWraith's own log destination
- `thresholds`: resource, SSH, Nginx, and website timeout thresholds
- `cooldowns`: per-alert cooldown periods
- `telegram.default_thread_id`: optional Telegram topic ID used when no specific route matches
- `telegram.alert_threads`: optional topic routes by exact alert type or purpose group
- `disk_paths`: filesystems to check
- `services`: systemd services to monitor
- `websites`: URLs to check
- `logs.ssh_auth`: usually `/var/log/auth.log` on Ubuntu/Debian
- `logs.nginx_access`: Nginx access log path
- `nginx_suspicious_paths`: path fragments that trigger web scan alerts

Telegram topic routing supports exact alert types first:

```json
"telegram": {
  "default_thread_id": 10,
  "alert_threads": {
    "ssh_login": 11,
    "ssh_bruteforce": 12,
    "website_down": 13
  }
}
```

It also supports broader purpose keys:

```json
"alert_threads": {
  "resource": 20,
  "service": 21,
  "website": 22,
  "ssh": 23,
  "web": 24
}
```

If `telegram` is omitted, alerts are sent to the main chat without `message_thread_id`.

## Run

Run once:

```bash
./build/cachewraith-monitor --config ./config.json --once
```

Run continuously:

```bash
./build/cachewraith-monitor --config ./config.json
```

Send a Telegram test alert:

```bash
./build/cachewraith-monitor --config ./config.json --test-telegram
```

Other CLI options:

```bash
./build/cachewraith-monitor --help
./build/cachewraith-monitor --version
```

## systemd Install

```bash
sudo cmake --install build --prefix /usr/local
sudo mkdir -p /etc/cachewraith-monitor
sudo cp config.example.json /etc/cachewraith-monitor/config.json
sudo cp systemd/cachewraith-monitor.service /etc/systemd/system/cachewraith-monitor.service
sudo systemctl daemon-reload
sudo systemctl enable --now cachewraith-monitor
sudo systemctl status cachewraith-monitor
```

The example service runs as `root` because `/var/log/auth.log` and service checks often require elevated permissions. You can use a dedicated user later if you grant read access to the needed logs and permissions for service checks.

## Example Alerts

```text
SSH LOGIN
Server: cachewraith-vps-01
User: root
IP: 1.2.3.4
Method: publickey
```

```text
SSH BRUTE FORCE
Server: cachewraith-vps-01
IP: 1.2.3.4
Failed Attempts: 15
Window: 5 minutes
```

```text
WEB SCAN DETECTED
Server: cachewraith-vps-01
IP: 1.2.3.4
Reason: suspicious path
Path: /.env
Count: 8
```

## Security Notes

- Telegram secrets are read only from environment variables.
- CacheWraith does not log Telegram tokens or chat IDs.
- Service names are validated before calling `systemctl`.
- `systemctl` is executed without a shell.
- HTTP checks use curl timeouts.
- Log tailing starts at end-of-file on first run to avoid flooding alerts from old logs.
- Keep `/etc/cachewraith-monitor/cachewraith.env` readable only by root.

## Roadmap v2

- Persistent log offsets across restarts
- Optional JSON structured logs
- Prometheus metrics endpoint
- Configurable per-monitor enable/disable flags
- Concurrent website checks with a bounded worker pool
- Journald backend for SSH detection on systems without auth.log
- More robust Nginx parser formats and allowlists
- Package recipes for Debian and Arch
