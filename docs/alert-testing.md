# Alert Testing

Use this guide after CacheWraith is installed and Telegram test alerts work.

Watch the log while testing:

```bash
sudo tail -f /var/log/cachewraith-monitor.log
```

Restart after config changes:

```bash
sudo systemctl restart cachewraith-monitor
```

## Service Down

Nginx is usually safe to stop briefly:

```bash
sudo systemctl stop nginx
sleep 70
sudo systemctl start nginx
```

Expected alert:

```text
SERVICE DOWN
Service: nginx
Status: not active
```

## Website Down

Temporarily add a fake website:

```json
"websites": [
  "https://example.invalid"
]
```

Restart and wait one interval.

Expected alert:

```text
WEBSITE HEALTH CHECK FAILED
```

## RAM Usage

Temporarily lower the threshold:

```json
"ram_percent": 1
```

Restart and wait one interval.

Restore it after testing:

```json
"ram_percent": 90
```

## Disk Usage

Temporarily lower:

```json
"disk_percent": 1
```

Restart and wait one interval.

Restore:

```json
"disk_percent": 90
```

## CPU Load

Temporarily lower:

```json
"load_average_1m": 0.0
```

Restart and wait one interval.

Restore:

```json
"load_average_1m": 4.0
```

## SSH Login

Open a second terminal and SSH into the VPS:

```bash
ssh your-vps-host
```

Expected alert:

```text
SSH LOGIN
```

## SSH Brute Force

Temporarily lower:

```json
"ssh_failed_attempts": 1
```

Restart, then make one failed SSH attempt from another machine:

```bash
ssh fakeuser@your-vps-ip
```

Expected alert:

```text
SSH BRUTE FORCE
```

Restore:

```json
"ssh_failed_attempts": 10
```

## Nginx Web Scan

From another machine:

```bash
curl http://your-vps-ip/.env
curl http://your-vps-ip/wp-admin
curl http://your-vps-ip/.git/config
```

Expected alert:

```text
WEB SCAN DETECTED
```

## Nginx 404 Burst

Temporarily lower:

```json
"nginx_404_per_minute": 2
```

Restart, then:

```bash
curl http://your-vps-ip/not-found-1
curl http://your-vps-ip/not-found-2
curl http://your-vps-ip/not-found-3
```

Expected alert:

```text
WEB 404 BURST
```

## Nginx Request Rate

Temporarily lower:

```json
"nginx_requests_per_minute": 3
```

Restart, then:

```bash
for i in {1..5}; do curl -s http://your-vps-ip/ >/dev/null; done
```

Expected alert:

```text
HIGH WEB REQUEST RATE
```

## Cleanup

Restore normal thresholds and restart:

```bash
sudo systemctl restart cachewraith-monitor
```
