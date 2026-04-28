package monitor

import (
	"context"
	"fmt"
	"regexp"
	"strconv"
	"strings"
	"time"

	"cachewraith-agent-go/internal/alert"
	"cachewraith-agent-go/internal/config"
	"cachewraith-agent-go/internal/logtailer"
)

type NginxAccessEvent struct {
	IP     string
	Method string
	Path   string
	Status int
}

var nginxCommon = regexp.MustCompile(`^([0-9a-fA-F:\.]+)\s+\S+\s+\S+\s+\[[^\]]+\]\s+"([A-Z]+)\s+([^"\s]+)[^"]*"\s+([0-9]{3})`)

func ParseNginxAccessLine(line string) (NginxAccessEvent, bool) {
	match := nginxCommon.FindStringSubmatch(line)
	if len(match) != 5 {
		return NginxAccessEvent{}, false
	}
	status, err := strconv.Atoi(match[4])
	if err != nil {
		return NginxAccessEvent{}, false
	}
	return NginxAccessEvent{IP: match[1], Method: match[2], Path: match[3], Status: status}, true
}

type NginxLog struct {
	cfg  config.Config
	tail *logtailer.Tailer
	byIP map[string]nginxCounter
}

type nginxCounter struct {
	Requests           int
	NotFound           int
	Suspicious         int
	LastSuspiciousPath string
	WindowStart        time.Time
}

func NewNginxLog(cfg config.Config) *NginxLog {
	return &NginxLog{cfg: cfg, tail: logtailer.New(cfg.Logs.NginxAccess), byIP: map[string]nginxCounter{}}
}

func (m *NginxLog) Name() string { return "nginx-log" }

func (m *NginxLog) Check(context.Context) []alert.Alert {
	var alerts []alert.Alert
	now := time.Now()
	for _, line := range m.tail.ReadNewLines() {
		event, ok := ParseNginxAccessLine(line)
		if !ok {
			continue
		}
		c := m.byIP[event.IP]
		if c.WindowStart.IsZero() || now.Sub(c.WindowStart) > time.Minute {
			c = nginxCounter{WindowStart: now}
		}
		c.Requests++
		if event.Status == httpNotFound {
			c.NotFound++
		}
		if suspiciousPath(event.Path, m.cfg.NginxSuspiciousPaths) {
			c.Suspicious++
			c.LastSuspiciousPath = event.Path
		}

		siteText := configuredWebsiteText(m.cfg.Websites)
		if c.Suspicious > 0 {
			alerts = append(alerts, alert.New("web_scan", event.IP+":suspicious", "WEB SCAN DETECTED", alert.Warning,
				fmt.Sprintf("Server: %s\nSource IP: %s\nWebsite URL: %s\nReason: suspicious path requested\nPath: %s\nCount: %d", m.cfg.ServerName, event.IP, siteText, c.LastSuspiciousPath, c.Suspicious)))
			c.Suspicious = 0
		}
		if c.Requests >= m.cfg.Thresholds.NginxRequestsPerMinute {
			alerts = append(alerts, alert.New("web_rate", event.IP, "HIGH WEB REQUEST RATE", alert.Warning,
				fmt.Sprintf("Server: %s\nSource IP: %s\nWebsite URL: %s\nRequests: %d\nWindow: 1 minute", m.cfg.ServerName, event.IP, siteText, c.Requests)))
			c.Requests = 0
		}
		if c.NotFound >= m.cfg.Thresholds.Nginx404PerMinute {
			alerts = append(alerts, alert.New("web_404", event.IP, "WEB 404 BURST", alert.Warning,
				fmt.Sprintf("Server: %s\nSource IP: %s\nWebsite URL: %s\n404 Responses: %d\nWindow: 1 minute", m.cfg.ServerName, event.IP, siteText, c.NotFound)))
			c.NotFound = 0
		}
		m.byIP[event.IP] = c
	}
	return alerts
}

const httpNotFound = 404

func suspiciousPath(path string, suspects []string) bool {
	lower := strings.ToLower(path)
	for _, suspect := range suspects {
		if strings.Contains(lower, strings.ToLower(suspect)) {
			return true
		}
	}
	return false
}

func configuredWebsiteText(websites []string) string {
	if len(websites) == 0 {
		return "none configured"
	}
	return strings.Join(websites, ", ")
}
