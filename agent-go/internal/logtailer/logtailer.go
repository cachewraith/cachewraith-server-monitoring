package logtailer

import (
	"bufio"
	"log/slog"
	"os"
)

type Tailer struct {
	path        string
	offset      int64
	lastSize    int64
	initialized bool
}

func New(path string) *Tailer {
	return &Tailer{path: path}
}

func (t *Tailer) ReadNewLines() []string {
	info, err := os.Stat(t.path)
	if err != nil {
		if t.initialized {
			slog.Warn("log file is missing or unreadable", "path", t.path, "error", err)
		}
		return nil
	}

	size := info.Size()
	if !t.initialized {
		t.offset = size
		t.lastSize = size
		t.initialized = true
		return nil
	}
	if size < t.lastSize {
		slog.Info("log rotation detected", "path", t.path)
		t.offset = 0
	}

	file, err := os.Open(t.path)
	if err != nil {
		slog.Warn("unable to read log file", "path", t.path, "error", err)
		return nil
	}
	defer file.Close()

	if _, err := file.Seek(t.offset, 0); err != nil {
		slog.Warn("unable to seek log file", "path", t.path, "error", err)
		return nil
	}

	var lines []string
	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		lines = append(lines, scanner.Text())
	}
	if pos, err := file.Seek(0, 1); err == nil {
		t.offset = pos
	} else {
		t.offset = size
	}
	t.lastSize = size
	return lines
}
