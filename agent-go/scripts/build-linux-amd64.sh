#!/usr/bin/env sh
set -eu

cd "$(dirname "$0")/.."
mkdir -p dist
GOOS=linux GOARCH=amd64 CGO_ENABLED=0 go build -trimpath -ldflags="-s -w" -o dist/monitor-agent ./cmd/agent
echo "Built dist/monitor-agent"
