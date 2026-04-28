package alert

import "time"

type Severity string

const (
	Info     Severity = "INFO"
	Warning  Severity = "WARNING"
	Critical Severity = "CRITICAL"
)

type Alert struct {
	Type           string
	Key            string
	Title          string
	Message        string
	Severity       Severity
	BypassCooldown bool
	CreatedAt      time.Time
}

func New(alertType, key, title string, severity Severity, message string) Alert {
	return Alert{
		Type:      alertType,
		Key:       key,
		Title:     title,
		Severity:  severity,
		Message:   message,
		CreatedAt: time.Now(),
	}
}
