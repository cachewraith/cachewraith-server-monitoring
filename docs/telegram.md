# Telegram Setup

CacheWraith sends Telegram alerts through a bot.

Secrets are read from environment variables:

```bash
CACHEWRAITH_TELEGRAM_BOT_TOKEN
CACHEWRAITH_TELEGRAM_CHAT_ID
```

Do not put the bot token in `config.json`.

## Create a Bot

1. Open Telegram.
2. Message `@BotFather`.
3. Run `/newbot`.
4. Copy the bot token.
5. Add the bot to your group.
6. Allow the bot to post messages.

If your group has topics enabled, the bot must be allowed to post in the target topics.

## Chat ID

For normal chats, use the chat ID from Telegram Bot API `getUpdates`.

For Telegram private supergroup links like:

```text
https://t.me/c/3960339223/14
```

The Bot API chat ID is usually:

```bash
-1003960339223
```

So:

```bash
export CACHEWRAITH_TELEGRAM_CHAT_ID="-1003960339223"
```

## Forum Topic IDs

For a topic link:

```text
https://t.me/c/3960339223/14
```

The topic ID is:

```text
14
```

To send all alerts to the main group or General topic, keep:

```json
"telegram": {
  "default_thread_id": null,
  "alert_threads": {}
}
```

To send all alerts to one topic:

```json
"telegram": {
  "default_thread_id": 14,
  "alert_threads": {}
}
```

To route different alert types:

```json
"telegram": {
  "default_thread_id": null,
  "alert_threads": {
    "resource": 20,
    "service": 21,
    "website": 22,
    "ssh": 23,
    "web": 24,
    "ssh_login": 25,
    "ssh_bruteforce": 26
  }
}
```

Exact alert type routes win before group routes.

## Supported Route Keys

Exact alert types:

- `ram_high`
- `disk_high`
- `load_high`
- `service_down`
- `website_down`
- `ssh_login`
- `ssh_bruteforce`
- `web_scan`
- `web_rate`
- `web_404`

Group routes:

- `resource`
- `service`
- `website`
- `ssh`
- `web`
- `default`

## Test Telegram

For local shell testing:

```bash
export CACHEWRAITH_TELEGRAM_BOT_TOKEN="your-bot-token"
export CACHEWRAITH_TELEGRAM_CHAT_ID="-1001234567890"
./build/cachewraith-monitor --config ./config.json --test-telegram
```

For production env file testing:

```bash
sudo env $(sudo cat /etc/cachewraith-monitor/cachewraith.env | xargs) \
  /usr/local/bin/cachewraith-monitor \
  --config /etc/cachewraith-monitor/config.json \
  --test-telegram
```

## Common Error

```text
Bad Request: message thread not found
```

This means the chat ID is valid, but the topic ID is wrong or the bot cannot access that topic.

Set `default_thread_id` to `null` and test again. Then add topic IDs one by one.
