#!/usr/bin/env bash
set -euo pipefail

HERE=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
COMPOSE=(docker compose -f "$HERE/compose.yaml")

build_image() {
    "${COMPOSE[@]}" build primary
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

run_functional() {
    start_primary
    "${COMPOSE[@]}" --profile functional run --rm functional-test functional
}

run_recovery_suite() {
    local suite=$1
    "${COMPOSE[@]}" --profile crash run --rm crash-test "$suite"
}

run_single_recovery_suite() {
    local suite=$1
    build_image
    run_recovery_suite "$suite"
}

run_crash() {
    build_image
    # Each invocation gets a fresh crash-test container with tmpfs PGDATA, so
    # build, committed-write, and in-flight recovery failures stay isolated.
    run_recovery_suite build
    run_recovery_suite committed
    run_recovery_suite inflight
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
    cat <<'EOF_USAGE'
Usage: ./run.sh COMMAND

Commands:
  build                Build Bingo from the selected source and create the PG17 test image
  normal               CREATE INDEX, incremental mutation, VACUUM, REINDEX, tail checks
  concurrent           Concurrent inserts plus non-key UPDATE regression
  functional           Repository's native bingo/tests pytest suite (--db postgres)
  recovery-build       Committed CREATE INDEX + immediate-stop recovery
  recovery-committed   Committed incremental writes + production-shaped non-HOT UPDATE recovery
  recovery-inflight    Immediate stop while INSERT/UPDATE/VACUUM is in flight
  crash                Run all three recovery suites above, each on fresh PGDATA
  replica              Fresh primary/physical-standby WAL replay test
  all                  Run normal, concurrent, functional, all recovery suites, and replica
  up                   Build and start the normal primary database
  shell                Open psql against the normal primary database
  logs                 Follow primary logs
  down                 Stop services and remove all test volumes
  status               Show Compose service status

Environment:
  BINGO_SOURCE_REF      WORKTREE (default) or a committed Git ref to build for A/B testing
EOF_USAGE
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
    functional)
        run_functional
        ;;
    recovery-build)
        run_single_recovery_suite build
        ;;
    recovery-committed)
        run_single_recovery_suite committed
        ;;
    recovery-inflight)
        run_single_recovery_suite inflight
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
        run_functional
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
        "${COMPOSE[@]}" --profile tools --profile functional --profile crash --profile replica down -v --remove-orphans
        ;;
    status)
        "${COMPOSE[@]}" --profile tools --profile functional --profile crash --profile replica ps
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
