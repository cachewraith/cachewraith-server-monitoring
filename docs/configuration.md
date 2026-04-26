# Configuration

CacheWraith reads a JSON config file.

Default path:

```bash
./config.json
```

Production path used by the systemd service:

```bash
/etc/cachewraith-monitor/config.json
```

## Minimal Starter Config

```json
{
  "server_name": "vultr",
  "interval_seconds": 60,
  "log_file": "/var/log/cachewraith-monitor.log",

  "thresholds": {
    "ram_percent": 90,
    "disk_percent": 90,
    "load_average_1m": 4.0,
    "ssh_failed_attempts": 10,
    "nginx_requests_per_minute": 120,
    "nginx_404_per_minute": 30,
    "website_timeout_seconds": 5
  },

  "cooldowns": {
    "default_seconds": 900,
    "resource_alert_seconds": 900,
    "website_alert_seconds": 300,
    "service_alert_seconds": 300,
    "ssh_bruteforce_seconds": 600,
    "web_scan_seconds": 600
  },

  "telegram": {
    "default_thread_id": null,
    "alert_threads": {}
  },

  "disk_paths": ["/"],
  "services": ["ssh", "nginx"],
  "websites": [],

  "logs": {
    "ssh_auth": "/var/log/auth.log",
    "nginx_access": "/var/log/nginx/access.log"
  },

  "nginx_suspicious_paths": [
    "/.env",
    "/wp-admin",
    "/xmlrpc.php",
    "/phpmyadmin",
    "/.git",
    "../",
    "union select",
    "<script"
  ]
}
```

## Important Fields

- `server_name`: label shown in every alert
- `interval_seconds`: how often monitors run
- `log_file`: CacheWraith's own log file
- `disk_paths`: filesystem paths to check
- `services`: systemd services to check
- `websites`: HTTP/HTTPS URLs to check
- `logs.ssh_auth`: SSH auth log path
- `logs.nginx_access`: Nginx access log path

## Service Names

Check service names on the VPS:

```bash
systemctl is-active ssh
systemctl is-active sshd
systemctl is-active nginx
```

Ubuntu/Debian commonly uses:

```json
"services": ["ssh", "nginx"]
```

Some systems use:

```json
"services": ["sshd", "nginx"]
```

Remove services you do not run.

## Websites

If you do not host a website yet:

```json
"websites": []
```

If you have websites:

```json
"websites": [
  "https://example.com",
  "https://api.example.com/health"
]
```

## Thresholds

Resource thresholds are intentionally simple:

- `ram_percent`: alert when memory usage reaches this percent
- `disk_percent`: alert when disk usage reaches this percent
- `load_average_1m`: alert when 1-minute load average reaches this value

Security thresholds:

- `ssh_failed_attempts`: failed SSH attempts from one IP in 5 minutes
- `nginx_requests_per_minute`: request burst from one IP
- `nginx_404_per_minute`: 404 burst from one IP

Website threshold:

- `website_timeout_seconds`: curl timeout for each website check

## Cooldowns

Cooldowns prevent repeated Telegram spam for the same alert key.

Examples:

- `service_down:nginx`
- `website_down:https://example.com`
- `ssh_bruteforce:1.2.3.4`

Successful SSH login alerts bypass cooldown by default.
