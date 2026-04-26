# Security Notes

CacheWraith is designed to avoid common monitoring-agent mistakes.

## Secrets

Telegram secrets are read from environment variables:

```bash
CACHEWRAITH_TELEGRAM_BOT_TOKEN
CACHEWRAITH_TELEGRAM_CHAT_ID
```

Do not store the bot token in `config.json`.

For systemd, store secrets here:

```text
/etc/cachewraith-monitor/cachewraith.env
```

Protect it:

```bash
sudo chmod 600 /etc/cachewraith-monitor/cachewraith.env
```

If a bot token is exposed in a screenshot, terminal history, or git commit, regenerate it with `@BotFather`.

## systemd Permissions

The example service runs as `root` because:

- `/var/log/auth.log` usually requires root or adm-group access
- service checks may need elevated permissions
- writing `/var/log/cachewraith-monitor.log` may require root

A dedicated service user is possible later, but it must have permission to read the required logs.

## Command Execution

Service checks validate service names before calling `systemctl`.

`systemctl` is executed without a shell, which avoids shell injection.

## Logs

The log tailer starts at end-of-file on first run. This prevents a new install from sending alerts for old historical log lines.

During runtime, it reads only new lines.

If a log file is missing or unreadable, CacheWraith logs a warning and continues.

## HTTP Requests

Telegram and website checks use curl timeouts to avoid hanging the agent for too long.

## Config Safety

Validate service names before adding them.

Good:

```json
"services": ["ssh", "nginx"]
```

Bad:

```json
"services": ["nginx; rm -rf /"]
```

The bad value will be rejected.
