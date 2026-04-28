package monitor

import (
	"context"
	"fmt"
	"log/slog"

	"cachewraith-agent-go/internal/alert"
	"cachewraith-agent-go/internal/config"
	"github.com/shirou/gopsutil/v4/disk"
	"github.com/shirou/gopsutil/v4/load"
	"github.com/shirou/gopsutil/v4/mem"
)

type RAM struct {
	cfg config.Config
}

func NewRAM(cfg config.Config) *RAM { return &RAM{cfg: cfg} }
func (m *RAM) Name() string         { return "ram" }

func (m *RAM) Check(ctx context.Context) []alert.Alert {
	vm, err := mem.VirtualMemoryWithContext(ctx)
	if err != nil {
		slog.Warn("unable to read memory stats", "error", err)
		return nil
	}
	used := int(vm.UsedPercent)
	if used < m.cfg.Thresholds.RAMPercent {
		return nil
	}
	return []alert.Alert{alert.New("ram_high", "memory", "RAM USAGE HIGH", alert.Critical,
		fmt.Sprintf("Server: %s\nUsage: %d%%\nThreshold: %d%%", m.cfg.ServerName, used, m.cfg.Thresholds.RAMPercent))}
}

type Disk struct {
	cfg config.Config
}

func NewDisk(cfg config.Config) *Disk { return &Disk{cfg: cfg} }
func (m *Disk) Name() string          { return "disk" }

func (m *Disk) Check(ctx context.Context) []alert.Alert {
	var alerts []alert.Alert
	for _, path := range m.cfg.DiskPaths {
		usage, err := disk.UsageWithContext(ctx, path)
		if err != nil {
			slog.Warn("unable to read disk stats", "path", path, "error", err)
			continue
		}
		used := int(usage.UsedPercent)
		if used >= m.cfg.Thresholds.DiskPercent {
			alerts = append(alerts, alert.New("disk_high", path, "DISK USAGE HIGH", alert.Critical,
				fmt.Sprintf("Server: %s\nPath: %s\nUsage: %d%%\nThreshold: %d%%", m.cfg.ServerName, path, used, m.cfg.Thresholds.DiskPercent)))
		}
	}
	return alerts
}

type CPU struct {
	cfg config.Config
}

func NewCPU(cfg config.Config) *CPU { return &CPU{cfg: cfg} }
func (m *CPU) Name() string         { return "cpu" }

func (m *CPU) Check(ctx context.Context) []alert.Alert {
	avg, err := load.AvgWithContext(ctx)
	if err != nil {
		slog.Warn("unable to read load average", "error", err)
		return nil
	}
	if avg.Load1 < m.cfg.Thresholds.LoadAverage1M {
		return nil
	}
	return []alert.Alert{alert.New("load_high", "loadavg1m", "CPU LOAD HIGH", alert.Warning,
		fmt.Sprintf("Server: %s\nLoad Average 1m: %.2f\nThreshold: %.2f", m.cfg.ServerName, avg.Load1, m.cfg.Thresholds.LoadAverage1M))}
}
