#!/usr/bin/env sh
set -eu

cd "$(dirname "$0")/.."

if [ ! -x dist/monitor-agent ]; then
  echo "dist/monitor-agent not found or not executable. Run scripts/build-linux-amd64.sh first." >&2
  exit 1
fi

install -m 0755 dist/monitor-agent /usr/local/bin/monitor-agent
install -m 0644 deploy/monitor-agent.service.example /etc/systemd/system/monitor-agent.service
mkdir -p /etc/monitor-agent

if [ ! -f /etc/monitor-agent.env ]; then
  install -m 0600 .env.example /etc/monitor-agent.env
  echo "Created /etc/monitor-agent.env from .env.example. Edit it before relying on the service."
fi
if [ ! -f /etc/monitor-agent/config.json ]; then
  install -m 0644 config.example.json /etc/monitor-agent/config.json
  echo "Created /etc/monitor-agent/config.json from config.example.json. Review it for this server."
fi

systemctl daemon-reload
systemctl enable monitor-agent
systemctl restart monitor-agent
systemctl status monitor-agent --no-pager
