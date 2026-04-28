package config

import (
	"encoding/json"
	"errors"
	"os"
	"path/filepath"
)

type Config struct {
	ServerName           string         `json:"server_name"`
	IntervalSeconds      int            `json:"interval_seconds"`
	LogFile              string         `json:"log_file"`
	Thresholds           Thresholds     `json:"thresholds"`
	Cooldowns            Cooldowns      `json:"cooldowns"`
	Telegram             TelegramTopics `json:"telegram"`
	DiskPaths            []string       `json:"disk_paths"`
	Services             []string       `json:"services"`
	Websites             []string       `json:"websites"`
	Logs                 LogPaths       `json:"logs"`
	NginxSuspiciousPaths []string       `json:"nginx_suspicious_paths"`
}

type Thresholds struct {
	RAMPercent             int     `json:"ram_percent"`
	DiskPercent            int     `json:"disk_percent"`
	LoadAverage1M          float64 `json:"load_average_1m"`
	SSHFailedAttempts      int     `json:"ssh_failed_attempts"`
	NginxRequestsPerMinute int     `json:"nginx_requests_per_minute"`
	Nginx404PerMinute      int     `json:"nginx_404_per_minute"`
	WebsiteTimeoutSeconds  int     `json:"website_timeout_seconds"`
}

type Cooldowns struct {
	DefaultSeconds       int `json:"default_seconds"`
	ResourceAlertSeconds int `json:"resource_alert_seconds"`
	WebsiteAlertSeconds  int `json:"website_alert_seconds"`
	ServiceAlertSeconds  int `json:"service_alert_seconds"`
	SSHBruteforceSeconds int `json:"ssh_bruteforce_seconds"`
	WebScanSeconds       int `json:"web_scan_seconds"`
}

type TelegramTopics struct {
	DefaultThreadID *int64           `json:"default_thread_id"`
	AlertThreads    map[string]int64 `json:"alert_threads"`
}

type LogPaths struct {
	SSHAuth     string `json:"ssh_auth"`
	NginxAccess string `json:"nginx_access"`
}

func Default() Config {
	return Config{
		ServerName:      "cachewraith-vps-01",
		IntervalSeconds: 60,
		LogFile:         "/var/log/cachewraith-monitor.log",
		Thresholds: Thresholds{
			RAMPercent:             90,
			DiskPercent:            90,
			LoadAverage1M:          4.0,
			SSHFailedAttempts:      10,
			NginxRequestsPerMinute: 120,
			Nginx404PerMinute:      30,
			WebsiteTimeoutSeconds:  5,
		},
		Cooldowns: Cooldowns{
			DefaultSeconds:       900,
			ResourceAlertSeconds: 900,
			WebsiteAlertSeconds:  300,
			ServiceAlertSeconds:  300,
			SSHBruteforceSeconds: 600,
			WebScanSeconds:       600,
		},
		Telegram:  TelegramTopics{AlertThreads: map[string]int64{}},
		DiskPaths: []string{"/"},
		Logs: LogPaths{
			SSHAuth:     "/var/log/auth.log",
			NginxAccess: "/var/log/nginx/access.log",
		},
		NginxSuspiciousPaths: []string{"/.env", "/wp-admin", "/xmlrpc.php", "/phpmyadmin", "/.git", "../", "union select", "<script"},
	}
}

func Load(path string) (Config, error) {
	cfg := Default()

	body, err := os.ReadFile(filepath.Clean(path))
	if err != nil {
		return cfg, err
	}
	if err := json.Unmarshal(body, &cfg); err != nil {
		return cfg, err
	}
	if cfg.Telegram.AlertThreads == nil {
		cfg.Telegram.AlertThreads = map[string]int64{}
	}
	return cfg, cfg.Validate()
}

func (c Config) Validate() error {
	if c.ServerName == "" {
		return errors.New("server_name must not be empty")
	}
	if c.IntervalSeconds <= 0 {
		return errors.New("interval_seconds must be greater than zero")
	}
	if c.Thresholds.RAMPercent < 1 || c.Thresholds.RAMPercent > 100 {
		return errors.New("thresholds.ram_percent must be between 1 and 100")
	}
	if c.Thresholds.DiskPercent < 1 || c.Thresholds.DiskPercent > 100 {
		return errors.New("thresholds.disk_percent must be between 1 and 100")
	}
	if c.Thresholds.WebsiteTimeoutSeconds < 1 || c.Thresholds.WebsiteTimeoutSeconds > 120 {
		return errors.New("thresholds.website_timeout_seconds must be between 1 and 120")
	}
	for _, service := range c.Services {
		if !IsSafeSystemdServiceName(service) {
			return errors.New("unsafe systemd service name: " + service)
		}
	}
	return nil
}

func IsSafeSystemdServiceName(name string) bool {
	if name == "" || len(name) > 128 {
		return false
	}
	for _, r := range name {
		if (r >= 'a' && r <= 'z') || (r >= 'A' && r <= 'Z') || (r >= '0' && r <= '9') || r == '_' || r == '-' || r == '.' || r == '@' {
			continue
		}
		return false
	}
	return true
}
