//go:build cgo && !libsqlite3 && !sqlite_nofts5
// +build cgo,!libsqlite3,!sqlite_nofts5

package sqlite3

import (
	"database/sql"
	"path/filepath"
	"testing"
)

// The yaklang fork enables FTS5 without opt-in tags. JSONB and that default
// must survive an amalgamation upgrade used by replace-based consumers.
func TestBundledJSONBAndDefaultFTS5(t *testing.T) {
	db, err := sql.Open("sqlite3", filepath.Join(t.TempDir(), "features.sqlite"))
	if err != nil {
		t.Fatal(err)
	}
	defer db.Close()
	var kind string
	var value int64
	if err := db.QueryRow(`SELECT typeof(jsonb('{"id":9007199254740993}')),
		json_extract(jsonb('{"id":9007199254740993}'),'$.id')`).Scan(&kind, &value); err != nil {
		t.Fatal(err)
	}
	if kind != "blob" || value != 9007199254740993 {
		t.Fatalf("JSONB compatibility: type=%q, integer=%d", kind, value)
	}
	if _, err := db.Exec(`CREATE VIRTUAL TABLE messages USING fts5(content);
		INSERT INTO messages(content) VALUES('pcap stream reassembly'),('other');`); err != nil {
		t.Fatal(err)
	}
	var matches int
	if err := db.QueryRow(`SELECT count(*) FROM messages WHERE messages MATCH 'pcap'`).Scan(&matches); err != nil {
		t.Fatal(err)
	}
	if matches != 1 {
		t.Fatalf("default FTS5 compatibility: matches=%d", matches)
	}
}
