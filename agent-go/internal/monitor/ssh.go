package monitor

import (
	"context"
	"fmt"
	"regexp"
	"time"

	"cachewraith-agent-go/internal/alert"
	"cachewraith-agent-go/internal/config"
	"cachewraith-agent-go/internal/logtailer"
)

type SSHEventKind string

const (
	SSHAcceptedPassword  SSHEventKind = "accepted_password"
	SSHAcceptedPublicKey SSHEventKind = "accepted_publickey"
	SSHFailedPassword    SSHEventKind = "failed_password"
)

type SSHEvent struct {
	Kind       SSHEventKind
	User       string
	IP         string
	Method     string
	DeviceName string
}

var (
	syslogPrefix = regexp.MustCompile(`^[A-Z][a-z]{2}\s+\d{1,2}\s+\d{2}:\d{2}:\d{2}\s+([^\s]+)`)
	sshInvalid   = regexp.MustCompile(`Failed password for invalid user ([^\s]+) from ([0-9a-fA-F:\.]+)`)
	sshFailed    = regexp.MustCompile(`Failed password for ([^\s]+) from ([0-9a-fA-F:\.]+)`)
	sshAccepted  = regexp.MustCompile(`Accepted (password|publickey) for ([^\s]+) from ([0-9a-fA-F:\.]+)`)
)

func ParseSSHLogLine(line string) (SSHEvent, bool) {
	device := ""
	if match := syslogPrefix.FindStringSubmatch(line); len(match) == 2 {
		device = match[1]
	}
	if match := sshAccepted.FindStringSubmatch(line); len(match) == 4 {
		kind := SSHAcceptedPublicKey
		if match[1] == "password" {
			kind = SSHAcceptedPassword
		}
		return SSHEvent{Kind: kind, User: match[2], IP: match[3], Method: match[1], DeviceName: device}, true
	}
	if match := sshInvalid.FindStringSubmatch(line); len(match) == 3 {
		return SSHEvent{Kind: SSHFailedPassword, User: match[1], IP: match[2], Method: "password", DeviceName: device}, true
	}
	if match := sshFailed.FindStringSubmatch(line); len(match) == 3 {
		return SSHEvent{Kind: SSHFailedPassword, User: match[1], IP: match[2], Method: "password", DeviceName: device}, true
	}
	return SSHEvent{}, false
}

type SSHLog struct {
	cfg        config.Config
	tailer     *logtailer.Tailer
	failedByIP map[string]counter
}

type counter struct {
	WindowStart time.Time
	Count       int
}

func NewSSHLog(cfg config.Config) *SSHLog {
	return &SSHLog{cfg: cfg, tailer: logtailer.New(cfg.Logs.SSHAuth), failedByIP: map[string]counter{}}
}

func (m *SSHLog) Name() string { return "ssh-log" }

func (m *SSHLog) Check(context.Context) []alert.Alert {
	var alerts []alert.Alert
	now := time.Now()
	for _, line := range m.tailer.ReadNewLines() {
		event, ok := ParseSSHLogLine(line)
		if !ok {
			continue
		}
		device := event.DeviceName
		if device == "" {
			device = m.cfg.ServerName
		}
		if event.Kind == SSHAcceptedPassword || event.Kind == SSHAcceptedPublicKey {
			a := alert.New("ssh_login", event.IP+":"+event.User+":"+event.Method, "SSH LOGIN SUCCESS", alert.Info,
				fmt.Sprintf("Device: %s\nConfigured Server: %s\nUser: %s\nFrom: %s\nAuth Method: %s", device, m.cfg.ServerName, event.User, event.IP, event.Method))
			a.BypassCooldown = true
			alerts = append(alerts, a)
			continue
		}

		c := m.failedByIP[event.IP]
		if c.WindowStart.IsZero() || now.Sub(c.WindowStart) > 5*time.Minute {
			c = counter{WindowStart: now}
		}
		c.Count++
		if c.Count >= m.cfg.Thresholds.SSHFailedAttempts {
			alerts = append(alerts, alert.New("ssh_bruteforce", event.IP, "SSH BRUTE FORCE DETECTED", alert.Critical,
				fmt.Sprintf("Device: %s\nConfigured Server: %s\nFrom: %s\nFailed Attempts: %d\nWindow: 5 minutes", device, m.cfg.ServerName, event.IP, c.Count)))
			c = counter{WindowStart: now}
		}
		m.failedByIP[event.IP] = c
	}
	return alerts
}
