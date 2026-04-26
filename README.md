# CacheWraith Server Monitoring

CacheWraith is a lightweight C++20 monitoring agent for Linux VPS servers. It checks server health, watches SSH and Nginx logs for suspicious activity, and sends alerts to Telegram.

It is designed for small production VPS deployments where you want useful alerts without installing a heavy monitoring stack.

## What It Monitors

- RAM usage
- Disk usage
- CPU load average
- systemd service status
- Website HTTP health
- SSH successful logins
- SSH failed login bursts
- Nginx suspicious requests
- Nginx high request rate
- Nginx 404 bursts

## Main Features

- Telegram alerts using libcurl
- Telegram forum topic routing with `message_thread_id`
- JSON config file
- Alert cooldowns to avoid spam
- Local log file for the agent
- Runtime log tailing, so huge old logs are not reread every loop
- systemd service example
- Basic parser and cooldown tests
- No secrets stored in config files

## Quick Start

Install dependencies on Ubuntu/Debian:

```bash
sudo apt update
sudo apt install -y build-essential cmake libcurl4-openssl-dev nlohmann-json3-dev
```

Build:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
```

Create a config:

```bash
cp config.example.json config.json
```

Set Telegram secrets in your shell:

```bash
export CACHEWRAITH_TELEGRAM_BOT_TOKEN="your-bot-token"
export CACHEWRAITH_TELEGRAM_CHAT_ID="your-chat-id"
```

Send a test alert:

```bash
./build/cachewraith-monitor --config ./config.json --test-telegram
```

Run one monitoring cycle:

```bash
./build/cachewraith-monitor --config ./config.json --once
```

Run continuously:

```bash
./build/cachewraith-monitor --config ./config.json
```

## Common VPS Install Layout

Recommended production paths:

```text
/opt/cachewraith/                         source code
/usr/local/bin/cachewraith-monitor        installed binary
/etc/cachewraith-monitor/config.json      production config
/etc/cachewraith-monitor/cachewraith.env  Telegram secrets
/var/log/cachewraith-monitor.log          agent log
```

Install with systemd:

```bash
sudo cmake --install build --prefix /usr/local
sudo mkdir -p /etc/cachewraith-monitor
sudo cp config.example.json /etc/cachewraith-monitor/config.json
sudo cp systemd/cachewraith-monitor.service /etc/systemd/system/cachewraith-monitor.service
sudo systemctl daemon-reload
sudo systemctl enable --now cachewraith-monitor
```

Check it:

```bash
sudo systemctl status cachewraith-monitor
sudo tail -f /var/log/cachewraith-monitor.log
```

## Documentation

- [Installation](docs/installation.md)
- [Configuration](docs/configuration.md)
- [Telegram Setup](docs/telegram.md)
- [Alert Testing](docs/alert-testing.md)
- [Architecture](docs/architecture.md)
- [Security Notes](docs/security.md)
- [Development](docs/development.md)
- [Roadmap](docs/roadmap.md)

## CLI

```bash
cachewraith-monitor --config /path/to/config.json
cachewraith-monitor --test-telegram
cachewraith-monitor --once
cachewraith-monitor --version
cachewraith-monitor --help
```

If `--config` is not provided, CacheWraith uses `./config.json`.

## License

No license has been added yet. Add one before publishing if other people should use or contribute to the project.
