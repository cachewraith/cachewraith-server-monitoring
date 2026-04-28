# CacheWraith Go Agent

This is a Go migration of the existing C++ CacheWraith monitoring agent. It keeps the same local-agent model for now: collect host signals, check thresholds, watch SSH and Nginx logs, apply cooldowns, and send Telegram alerts.

Laravel and dashboard integration are intentionally not included yet.

## What It Monitors

- RAM usage
- Disk usage for configured paths
- CPU 1-minute load average
- systemd service status
- Website HTTP health
- SSH successful logins
- SSH failed login bursts
- Nginx suspicious requests
- Nginx high request rate
- Nginx 404 bursts

## Configuration

Copy and edit:

```bash
cp config.example.json config.json
cp .env.example .env
```

Telegram secrets are read from environment variables:

```bash
export CACHEWRAITH_TELEGRAM_BOT_TOKEN="your-bot-token"
export CACHEWRAITH_TELEGRAM_CHAT_ID="your-chat-id"
```

The JSON config mirrors the C++ agent config closely, including thresholds, cooldowns, `disk_paths`, `services`, `websites`, log paths, and Telegram thread routing.

## Run Locally

```bash
go run ./cmd/agent --config ./config.json --once
go run ./cmd/agent --config ./config.json
go run ./cmd/agent --config ./config.json --test-telegram
```

## Build

```bash
make test
make build
```

Or build a Linux amd64 binary:

```bash
scripts/build-linux-amd64.sh
```

The binary is written to:

```text
dist/monitor-agent
```

## Install With systemd

```bash
scripts/build-linux-amd64.sh
sudo scripts/install-systemd.sh
sudo nano /etc/monitor-agent.env
sudo nano /etc/monitor-agent/config.json
sudo systemctl restart monitor-agent
```

Check status and logs:

```bash
sudo systemctl status monitor-agent
sudo journalctl -u monitor-agent -f
sudo tail -f /var/log/cachewraith-monitor-go.log
```

Uninstall:

```bash
sudo scripts/uninstall-systemd.sh
```

## Migration Notes

The Go agent currently targets feature parity with the C++ agent, not the Laravel API. It intentionally keeps secrets out of JSON config and uses environment variables for Telegram credentials.

The service monitor validates systemd service names and calls `systemctl` without a shell. Log tailers start at end-of-file on first run, matching the C++ behavior that avoids replaying historical logs.
