#!/usr/bin/env bash
set -euo pipefail

HERE=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
COMPOSE=(docker compose -f "$HERE/compose.yaml")
BINGO_SO="$HERE/libbingo-postgres.so"

require_so() {
    if [[ ! -f "$BINGO_SO" ]]; then
        cat >&2 <<EOF
Missing $BINGO_SO

Copy the patched PostgreSQL 17 library you want to test there first, for example:
  cp /path/to/libbingo-postgres.so "$BINGO_SO"
EOF
        exit 2
    fi
}

build_image() {
    require_so
    "${COMPOSE[@]}" build
}

start_primary() {
    build_image
    "${COMPOSE[@]}" up -d primary
}

run_normal() {
    start_primary
    "${COMPOSE[@]}" --profile tools run --rm runner normal
}

run_concurrent() {
    start_primary
    "${COMPOSE[@]}" --profile tools run --rm runner concurrent
}

run_crash() {
    build_image
    "${COMPOSE[@]}" --profile crash run --rm crash-test
}

run_replica() {
    build_image
    # Always take a fresh base backup. A standby data directory from an older
    # primary lifecycle is not useful for a deterministic WAL regression test.
    "${COMPOSE[@]}" --profile replica down -v --remove-orphans
    "${COMPOSE[@]}" --profile replica up -d primary standby
    "${COMPOSE[@]}" --profile replica run --rm replica-test replica
}

usage() {
    cat <<'EOF'
Usage: ./run.sh COMMAND

Commands:
  build        Build the PostgreSQL 17 + patched Bingo test image
  normal       CREATE INDEX, incremental mutation, VACUUM, REINDEX, tail checks
  concurrent   Concurrent inserts plus non-key UPDATE regression
  crash        Disposable immediate-stop crash/recovery test
  replica      Fresh primary/physical-standby WAL replay test
  all          Run normal, concurrent, crash, and replica tests
  up           Start the normal primary database
  shell        Open psql against the normal primary database
  logs         Follow primary logs
  down         Stop services and remove all test volumes
  status       Show Compose service status
EOF
}

command=${1:-}
case "$command" in
    build)
        build_image
        ;;
    normal)
        run_normal
        ;;
    concurrent)
        run_concurrent
        ;;
    crash)
        run_crash
        ;;
    replica)
        run_replica
        ;;
    all)
        run_normal
        run_concurrent
        run_crash
        run_replica
        ;;
    up)
        start_primary
        ;;
    shell)
        "${COMPOSE[@]}" exec primary psql -U postgres -d test
        ;;
    logs)
        "${COMPOSE[@]}" logs -f primary
        ;;
    down)
        "${COMPOSE[@]}" --profile tools --profile crash --profile replica down -v --remove-orphans
        ;;
    status)
        "${COMPOSE[@]}" --profile tools --profile crash --profile replica ps
        ;;
    -h|--help|help|"")
        usage
        ;;
    *)
        echo "Unknown command: $command" >&2
        usage >&2
        exit 2
        ;;
esac
