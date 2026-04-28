#!/usr/bin/env sh
set -eu

systemctl stop monitor-agent 2>/dev/null || true
systemctl disable monitor-agent 2>/dev/null || true
rm -f /etc/systemd/system/monitor-agent.service
rm -f /usr/local/bin/monitor-agent
systemctl daemon-reload
echo "monitor-agent service and binary removed. /etc/monitor-agent.env and /etc/monitor-agent/config.json were left in place."
