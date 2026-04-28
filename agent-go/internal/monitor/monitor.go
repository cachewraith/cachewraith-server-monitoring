package monitor

import (
	"context"

	"cachewraith-agent-go/internal/alert"
)

type Monitor interface {
	Name() string
	Check(context.Context) []alert.Alert
}
