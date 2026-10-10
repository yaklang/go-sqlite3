/*
** Adapted from SQLite's test/walrestart.test (2026-03-03).
** The author disclaims copyright to this source code. In place of a legal
** notice, here is a blessing:
**   May you do good and not evil.
**   May you find forgiveness for yourself and forgive others.
**   May you share freely, never taking more than you give.
**
** Fault point 660 resets the WAL after the checkpointer reads its header,
** before it reads nBackfill. This tests the real bundled C core, without
** depending on a low-probability scheduling race or Go's race detector.
*/
#include "sqlite3-binding.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static sqlite3 *writer;
static int injected;

static void execute(sqlite3 *db, const char *sql) {
  char *error = 0;
  int rc = sqlite3_exec(db, sql, 0, 0, &error);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "%s: %s\n", sql, error ? error : sqlite3_errmsg(db));
    sqlite3_free(error);
    exit(1);
  }
}

static char *scalar(sqlite3 *db, const char *sql) {
  sqlite3_stmt *statement = 0;
  char *result;
  if (sqlite3_prepare_v2(db, sql, -1, &statement, 0) != SQLITE_OK ||
      sqlite3_step(statement) != SQLITE_ROW) {
    fprintf(stderr, "%s: %s\n", sql, sqlite3_errmsg(db));
    exit(1);
  }
  result = sqlite3_mprintf("%s", sqlite3_column_text(statement, 0));
  if (!result || sqlite3_finalize(statement) != SQLITE_OK) exit(1);
  return result;
}

static int fault(int point) {
  if (point == 660 && !injected) {
    injected = 1;
    execute(writer, "UPDATE t1 SET b=randomblob(600) WHERE a<5");
  }
  return 0;
}

int main(int argc, char **argv) {
  sqlite3 *checkpoint = 0;
  char *expected, *actual, *integrity;
  const char *content =
      "SELECT group_concat(a||':'||hex(b),'|') "
      "FROM (SELECT a,b FROM t1 ORDER BY a)";
  if (argc != 2) return 1;
  if (sqlite3_open(argv[1], &checkpoint) != SQLITE_OK) return 1;
  execute(checkpoint,
      "PRAGMA page_size=1024; PRAGMA auto_vacuum=0;"
      "PRAGMA journal_mode=WAL; PRAGMA synchronous=FULL;"
      "PRAGMA wal_autocheckpoint=0;"
      "CREATE TABLE t1(a INTEGER PRIMARY KEY,b);"
      "WITH RECURSIVE s(i) AS (SELECT 1 UNION ALL SELECT i+1 FROM s WHERE i<20) "
      "INSERT INTO t1 SELECT NULL,randomblob(600) FROM s;"
      "CREATE INDEX i1 ON t1(b); PRAGMA wal_checkpoint;"
      "UPDATE t1 SET b=randomblob(600); PRAGMA wal_checkpoint;");
  if (sqlite3_open(argv[1], &writer) != SQLITE_OK) return 1;
  execute(writer, "PRAGMA synchronous=FULL; PRAGMA wal_autocheckpoint=0;");
  if (sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, fault) != SQLITE_OK)
    return 1;
  execute(checkpoint, "PRAGMA wal_checkpoint");
  sqlite3_test_control(SQLITE_TESTCTRL_FAULT_INSTALL, (int (*)(int))0);
  if (!injected) {
    fprintf(stderr, "WAL fault point 660 did not execute\n");
    return 1;
  }
  execute(writer, "UPDATE t1 SET b=randomblob(600)");
  expected = scalar(writer, content);
  execute(checkpoint, "PRAGMA wal_checkpoint");
  if (sqlite3_close(writer) != SQLITE_OK ||
      sqlite3_close(checkpoint) != SQLITE_OK) return 1;
  if (sqlite3_open(argv[1], &checkpoint) != SQLITE_OK) return 1;
  integrity = scalar(checkpoint, "PRAGMA integrity_check");
  actual = scalar(checkpoint, content);
  if (strcmp(integrity, "ok") || strcmp(actual, expected)) {
    fprintf(stderr, "WAL restart lost committed content: integrity=%s\n", integrity);
    return 1;
  }
  sqlite3_free(integrity);
  sqlite3_free(expected);
  sqlite3_free(actual);
  if (sqlite3_close(checkpoint) != SQLITE_OK) return 1;
  return 0;
}
