package alert

import (
	"context"
	"fmt"
	"io"
	"log/slog"
	"net/http"
	"net/url"
	"os"
	"strings"
	"time"
)

type TelegramNotifier struct {
	token  string
	chatID string
	client *http.Client
}

func NewTelegramNotifier() *TelegramNotifier {
	return &TelegramNotifier{
		token:  strings.TrimSpace(os.Getenv("CACHEWRAITH_TELEGRAM_BOT_TOKEN")),
		chatID: strings.TrimSpace(os.Getenv("CACHEWRAITH_TELEGRAM_CHAT_ID")),
		client: &http.Client{Timeout: 8 * time.Second},
	}
}

func (n *TelegramNotifier) Configured() bool {
	return n.token != "" && n.chatID != ""
}

func (n *TelegramNotifier) Send(ctx context.Context, text string, threadID *int64) error {
	if !n.Configured() {
		return fmt.Errorf("telegram is not configured; set CACHEWRAITH_TELEGRAM_BOT_TOKEN and CACHEWRAITH_TELEGRAM_CHAT_ID")
	}

	form := url.Values{}
	form.Set("chat_id", n.chatID)
	form.Set("disable_web_page_preview", "true")
	form.Set("text", text)
	if threadID != nil {
		form.Set("message_thread_id", fmt.Sprintf("%d", *threadID))
	}

	endpoint := "https://api.telegram.org/bot" + n.token + "/sendMessage"
	req, err := http.NewRequestWithContext(ctx, http.MethodPost, endpoint, strings.NewReader(form.Encode()))
	if err != nil {
		return err
	}
	req.Header.Set("Content-Type", "application/x-www-form-urlencoded")
	req.Header.Set("User-Agent", "cachewraith-monitor-go/0.1")

	resp, err := n.client.Do(req)
	if err != nil {
		return err
	}
	defer resp.Body.Close()
	if resp.StatusCode < 200 || resp.StatusCode >= 300 {
		body, _ := io.ReadAll(io.LimitReader(resp.Body, 4096))
		return fmt.Errorf("telegram returned HTTP %d: %s", resp.StatusCode, string(body))
	}
	return nil
}

func (n *TelegramNotifier) SendTest(ctx context.Context, serverName string, threadID *int64) error {
	return n.Send(ctx, "CacheWraith test alert\nServer: "+serverName+"\nStatus: Telegram notifications are working", threadID)
}

type Manager struct {
	cfg      configView
	notifier *TelegramNotifier
	cooldown *CooldownManager
}

type configView interface {
	ThreadID(Alert) *int64
}

func NewManager(cfg configView, notifier *TelegramNotifier, cooldown *CooldownManager) *Manager {
	return &Manager{cfg: cfg, notifier: notifier, cooldown: cooldown}
}

func (m *Manager) Handle(ctx context.Context, alerts []Alert) {
	for _, a := range alerts {
		slog.Warn("alert generated", "type", a.Type, "key", a.Key, "title", a.Title)
		if !m.cooldown.ShouldSend(a) {
			slog.Info("alert suppressed by cooldown", "type", a.Type, "key", a.Key)
			continue
		}
		if err := m.notifier.Send(ctx, format(a), m.cfg.ThreadID(a)); err != nil {
			slog.Error("failed to send telegram alert", "type", a.Type, "key", a.Key, "error", err)
		}
	}
}

func format(a Alert) string {
	text := fmt.Sprintf("[%s] %s\nTime: %s\n%s", a.Severity, a.Title, a.CreatedAt.Format("2006-01-02 15:04:05 MST"), a.Message)
	if action := action(a.Type); action != "" {
		text += "\nAction: " + action
	}
	return text
}

func action(alertType string) string {
	switch alertType {
	case "ssh_login":
		return "Verify this user and source IP. Rotate keys/passwords if it was not expected."
	case "ssh_bruteforce":
		return "Block or rate-limit the source IP and review SSH hardening."
	case "web_scan":
		return "Check the request in Nginx logs and block the source IP if it keeps scanning."
	case "web_rate":
		return "Inspect traffic from this IP and apply rate limiting if needed."
	case "web_404":
		return "Review requested paths and block noisy scanners if confirmed."
	case "service_down":
		return "Restart or inspect the service with systemctl and journalctl."
	case "website_down":
		return "Check the site endpoint, upstream service, DNS, and network path."
	case "ram_high", "disk_high", "load_high":
		return "Inspect the busiest processes and free capacity before the host degrades."
	default:
		return ""
	}
}
