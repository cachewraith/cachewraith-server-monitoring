package main

import (
	"context"
	"flag"
	"log/slog"
	"os"
	"os/signal"
	"syscall"
	"time"

	"cachewraith-agent-go/internal/alert"
	"cachewraith-agent-go/internal/config"
	"cachewraith-agent-go/internal/logger"
	"cachewraith-agent-go/internal/monitor"
	"cachewraith-agent-go/internal/version"
)

func main() {
	configPath := flag.String("config", "./config.json", "load config from path")
	once := flag.Bool("once", false, "run one monitoring cycle and exit")
	testTelegram := flag.Bool("test-telegram", false, "send a test Telegram alert and exit")
	showVersion := flag.Bool("version", false, "print version and exit")
	flag.Parse()

	if *showVersion {
		os.Stdout.WriteString("cachewraith-monitor-go " + version.Version + "\n")
		return
	}

	cfg, err := config.Load(*configPath)
	if err != nil {
		slog.Error("configuration error", "path", *configPath, "error", err)
		os.Exit(1)
	}

	slog.SetDefault(logger.New(cfg.LogFile))
	notifier := alert.NewTelegramNotifier()

	if *testTelegram {
		if err := notifier.SendTest(context.Background(), cfg.ServerName, cfg.Telegram.DefaultThreadID); err != nil {
			slog.Error("telegram test failed", "error", err)
			os.Exit(1)
		}
		return
	}

	ctx, stop := signal.NotifyContext(context.Background(), syscall.SIGINT, syscall.SIGTERM)
	defer stop()

	app := newApplication(cfg, notifier)
	if *once {
		app.runOnce(ctx)
		return
	}

	app.run(ctx)
}

type scheduledMonitor struct {
	monitor          monitor.Monitor
	intervalSeconds  int
	secondsUntilNext int
}

type application struct {
	cfg      config.Config
	manager  *alert.Manager
	monitors []scheduledMonitor
}

func newApplication(cfg config.Config, notifier *alert.TelegramNotifier) *application {
	app := &application{
		cfg:     cfg,
		manager: alert.NewManager(threadRouter{cfg: cfg}, notifier, alert.NewCooldownManager(cfg.Cooldowns)),
	}
	app.add(monitor.NewSSHLog(cfg), 1)
	app.add(monitor.NewNginxLog(cfg), 1)
	app.add(monitor.NewRAM(cfg), cfg.IntervalSeconds)
	app.add(monitor.NewDisk(cfg), cfg.IntervalSeconds)
	app.add(monitor.NewCPU(cfg), cfg.IntervalSeconds)
	app.add(monitor.NewService(cfg), cfg.IntervalSeconds)
	app.add(monitor.NewWebsite(cfg), cfg.IntervalSeconds)
	return app
}

func (a *application) add(m monitor.Monitor, intervalSeconds int) {
	if intervalSeconds < 1 {
		intervalSeconds = 1
	}
	a.monitors = append(a.monitors, scheduledMonitor{monitor: m, intervalSeconds: intervalSeconds})
}

func (a *application) runOnce(ctx context.Context) {
	var alerts []alert.Alert
	for i := range a.monitors {
		alerts = append(alerts, a.safeCheck(ctx, a.monitors[i].monitor)...)
	}
	a.manager.Handle(ctx, alerts)
}

func (a *application) run(ctx context.Context) {
	slog.Info("CacheWraith Go monitor started", "version", version.Version)
	ticker := time.NewTicker(time.Second)
	defer ticker.Stop()
	for {
		for i := range a.monitors {
			if a.monitors[i].secondsUntilNext > 0 {
				a.monitors[i].secondsUntilNext--
				continue
			}
			a.manager.Handle(ctx, a.safeCheck(ctx, a.monitors[i].monitor))
			a.monitors[i].secondsUntilNext = a.monitors[i].intervalSeconds - 1
		}
		select {
		case <-ctx.Done():
			slog.Info("CacheWraith Go monitor stopped")
			return
		case <-ticker.C:
		}
	}
}

func (a *application) safeCheck(ctx context.Context, m monitor.Monitor) (alerts []alert.Alert) {
	defer func() {
		if recovered := recover(); recovered != nil {
			slog.Error("monitor panicked", "monitor", m.Name(), "panic", recovered)
			alerts = nil
		}
	}()
	return m.Check(ctx)
}

type threadRouter struct {
	cfg config.Config
}

func (r threadRouter) ThreadID(a alert.Alert) *int64 {
	if id, ok := r.cfg.Telegram.AlertThreads[a.Type]; ok {
		return &id
	}
	category := ""
	switch a.Type {
	case "ram_high", "disk_high", "load_high":
		category = "resource"
	case "service_down":
		category = "service"
	case "website_down":
		category = "website"
	case "ssh_login", "ssh_bruteforce":
		category = "ssh"
	case "web_scan", "web_rate", "web_404":
		category = "web"
	}
	if category != "" {
		if id, ok := r.cfg.Telegram.AlertThreads[category]; ok {
			return &id
		}
	}
	if id, ok := r.cfg.Telegram.AlertThreads["default"]; ok {
		return &id
	}
	return r.cfg.Telegram.DefaultThreadID
}
