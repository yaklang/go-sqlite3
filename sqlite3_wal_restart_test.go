//go:build cgo && !libsqlite3 && !windows
// +build cgo,!libsqlite3,!windows

package sqlite3

import (
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"testing"
)

func TestBundledWALRestartRegression(t *testing.T) {
	version, number, _ := Version()
	if number < 3051003 {
		t.Fatalf("bundled SQLite %s predates the WAL-reset fix in 3.51.3", version)
	}
	compiler := strings.Fields(os.Getenv("CC"))
	if len(compiler) == 0 {
		compiler = []string{"cc"}
	}
	root := t.TempDir()
	binary := filepath.Join(root, "wal-restart")
	args := append(compiler[1:], "-std=gnu99", "-DSQLITE_THREADSAFE=1", "-I.",
		"testdata/wal_restart_regression.c", "sqlite3-binding.c", "-o", binary,
		"-lpthread", "-ldl", "-lm")
	if output, err := exec.Command(compiler[0], args...).CombinedOutput(); err != nil {
		t.Fatalf("compile WAL restart regression: %v\n%s", err, output)
	}
	if output, err := exec.Command(binary, filepath.Join(root, "capture.sqlite")).CombinedOutput(); err != nil {
		t.Fatalf("WAL restart regression: %v\n%s", err, output)
	}
}
