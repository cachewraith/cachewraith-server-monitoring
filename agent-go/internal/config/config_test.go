package config

import (
	"os"
	"path/filepath"
	"testing"
)

func TestLoadDefaults(t *testing.T) {
	path := filepath.Join(t.TempDir(), "config.json")
	if err := os.WriteFile(path, []byte(`{"server_name":"srv-1"}`), 0600); err != nil {
		t.Fatal(err)
	}
	cfg, err := Load(path)
	if err != nil {
		t.Fatalf("Load() error = %v", err)
	}
	if cfg.IntervalSeconds != 60 {
		t.Fatalf("IntervalSeconds = %d, want 60", cfg.IntervalSeconds)
	}
	if len(cfg.DiskPaths) != 1 || cfg.DiskPaths[0] != "/" {
		t.Fatalf("DiskPaths = %#v, want [/]", cfg.DiskPaths)
	}
	if cfg.Thresholds.RAMPercent != 90 {
		t.Fatalf("RAMPercent = %d, want 90", cfg.Thresholds.RAMPercent)
	}
}

func TestUnsafeServiceName(t *testing.T) {
	cfg := Default()
	cfg.Services = []string{"nginx;rm"}
	if err := cfg.Validate(); err == nil {
		t.Fatal("Validate() error = nil, want unsafe service error")
	}
}
