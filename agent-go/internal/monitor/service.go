package monitor

import (
	"context"
	"fmt"
	"log/slog"
	"os/exec"

	"cachewraith-agent-go/internal/alert"
	"cachewraith-agent-go/internal/config"
)

type Service struct {
	cfg config.Config
}

func NewService(cfg config.Config) *Service { return &Service{cfg: cfg} }
func (m *Service) Name() string             { return "service" }

func (m *Service) Check(ctx context.Context) []alert.Alert {
	var alerts []alert.Alert
	for _, service := range m.cfg.Services {
		if !config.IsSafeSystemdServiceName(service) {
			slog.Warn("skipping unsafe service name", "service", service)
			continue
		}
		if err := exec.CommandContext(ctx, "systemctl", "is-active", "--quiet", service).Run(); err != nil {
			alerts = append(alerts, alert.New("service_down", service, "SERVICE DOWN", alert.Critical,
				fmt.Sprintf("Server: %s\nService: %s\nStatus: not active", m.cfg.ServerName, service)))
		}
	}
	return alerts
}
