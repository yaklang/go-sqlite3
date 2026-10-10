FROM gcr.io/oss-fuzz-base/base-builder-go

COPY . $SRC/go-sqlite3
WORKDIR $SRC/go-sqlite3
COPY .clusterfuzzlite/build.sh $SRC/build.sh
