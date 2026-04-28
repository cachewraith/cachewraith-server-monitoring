package alert

import (
	"time"

	"cachewraith-agent-go/internal/config"
)

type CooldownManager struct {
	config config.Cooldowns
	sentAt map[string]time.Time
}

func NewCooldownManager(cfg config.Cooldowns) *CooldownManager {
	return &CooldownManager{config: cfg, sentAt: map[string]time.Time{}}
}

func (m *CooldownManager) ShouldSend(a Alert) bool {
	if a.BypassCooldown {
		return true
	}
	key := a.Type + ":" + a.Key
	now := time.Now()
	if last, ok := m.sentAt[key]; ok && now.Sub(last) < m.duration(a.Type) {
		return false
	}
	m.sentAt[key] = now
	return true
}

func (m *CooldownManager) duration(alertType string) time.Duration {
	seconds := m.config.DefaultSeconds
	switch alertType {
	case "ram_high", "disk_high", "load_high":
		seconds = m.config.ResourceAlertSeconds
	case "website_down":
		seconds = m.config.WebsiteAlertSeconds
	case "service_down":
		seconds = m.config.ServiceAlertSeconds
	case "ssh_bruteforce":
		seconds = m.config.SSHBruteforceSeconds
	case "web_scan", "web_rate", "web_404":
		seconds = m.config.WebScanSeconds
	}
	if seconds <= 0 {
		seconds = 900
	}
	return time.Duration(seconds) * time.Second
}
