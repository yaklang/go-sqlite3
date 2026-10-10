FROM gcr.io/oss-fuzz-base/base-builder-go

COPY . $GOPATH/src/github.com/yaklang/go-sqlite3
WORKDIR $GOPATH/src/github.com/yaklang/go-sqlite3
COPY .clusterfuzzlite/build.sh $SRC/build.sh
