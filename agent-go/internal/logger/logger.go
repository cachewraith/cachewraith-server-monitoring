package logger

import (
	"io"
	"log/slog"
	"os"
	"path/filepath"
)

func New(path string) *slog.Logger {
	writer := io.Writer(os.Stderr)
	if path != "" {
		if err := os.MkdirAll(filepath.Dir(path), 0755); err == nil {
			if file, err := os.OpenFile(path, os.O_CREATE|os.O_APPEND|os.O_WRONLY, 0644); err == nil {
				writer = file
			}
		}
	}
	return slog.New(slog.NewTextHandler(writer, &slog.HandlerOptions{Level: slog.LevelInfo}))
}
