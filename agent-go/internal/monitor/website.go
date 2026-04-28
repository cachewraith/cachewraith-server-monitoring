package monitor

import (
	"context"
	"fmt"
	"net/http"
	"time"

	"cachewraith-agent-go/internal/alert"
	"cachewraith-agent-go/internal/config"
)

type Website struct {
	cfg config.Config
}

func NewWebsite(cfg config.Config) *Website { return &Website{cfg: cfg} }
func (m *Website) Name() string             { return "website" }

func (m *Website) Check(ctx context.Context) []alert.Alert {
	var alerts []alert.Alert
	timeout := time.Duration(m.cfg.Thresholds.WebsiteTimeoutSeconds) * time.Second
	client := &http.Client{Timeout: timeout}
	for _, target := range m.cfg.Websites {
		req, err := http.NewRequestWithContext(ctx, http.MethodGet, target, nil)
		if err == nil {
			req.Header.Set("User-Agent", "cachewraith-monitor-go/0.1")
		}
		var result string
		if err != nil {
			result = err.Error()
		} else if resp, err := client.Do(req); err != nil {
			result = err.Error()
		} else {
			result = fmt.Sprintf("HTTP %d", resp.StatusCode)
			resp.Body.Close()
			if resp.StatusCode >= 200 && resp.StatusCode < 400 {
				continue
			}
		}

		alerts = append(alerts, alert.New("website_down", target, "WEBSITE HEALTH CHECK FAILED", alert.Critical,
			fmt.Sprintf("Server: %s\nURL: %s\nResult: %s", m.cfg.ServerName, target, result)))
	}
	return alerts
}
