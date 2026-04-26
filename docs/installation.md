# Installation

This guide installs CacheWraith on a Linux VPS.

## Recommended Location

Use `/opt` for the source code:

```bash
sudo mkdir -p /opt/cachewraith
cd /opt/cachewraith
sudo git clone https://github.com/YOUR_USER/cachewraith-server-monitoring.git .
```

Do not use `/var/www` unless this project is being served as a website. CacheWraith is a system agent, so `/opt` is a better fit.

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

If `nlohmann-json` is unavailable, CMake can fetch it during configure.

## Build

```bash
cd /opt/cachewraith
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
```

The binary will be:

```bash
./build/cachewraith-monitor
```

## Production Config

Create the config directory:

```bash
sudo mkdir -p /etc/cachewraith-monitor
sudo cp config.example.json /etc/cachewraith-monitor/config.json
sudo nano /etc/cachewraith-monitor/config.json
```

Create the Telegram environment file:

```bash
sudo nano /etc/cachewraith-monitor/cachewraith.env
```

Example:

```bash
CACHEWRAITH_TELEGRAM_BOT_TOKEN=your-bot-token
CACHEWRAITH_TELEGRAM_CHAT_ID=-1001234567890
```

Protect it:

```bash
sudo chmod 600 /etc/cachewraith-monitor/cachewraith.env
```

## Manual Test

Load the env file and send a test alert:

```bash
sudo env $(sudo cat /etc/cachewraith-monitor/cachewraith.env | xargs) \
  ./build/cachewraith-monitor \
  --config /etc/cachewraith-monitor/config.json \
  --test-telegram
```

Run one monitoring cycle:

```bash
sudo env $(sudo cat /etc/cachewraith-monitor/cachewraith.env | xargs) \
  ./build/cachewraith-monitor \
  --config /etc/cachewraith-monitor/config.json \
  --once
```

## systemd Install

Install the binary:

```bash
sudo cmake --install build --prefix /usr/local
```

Install the service:

```bash
sudo cp systemd/cachewraith-monitor.service /etc/systemd/system/cachewraith-monitor.service
sudo systemctl daemon-reload
sudo systemctl enable --now cachewraith-monitor
```

Check status:

```bash
sudo systemctl status cachewraith-monitor
sudo journalctl -u cachewraith-monitor -n 50
sudo tail -f /var/log/cachewraith-monitor.log
```

## Reboot Check

After installation, reboot once and confirm the service starts automatically:

```bash
sudo reboot
```

After reconnecting:

```bash
sudo systemctl status cachewraith-monitor
```
