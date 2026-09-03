#!/bin/bash
# Strips DISABLE_OOB=ON from sqlnet.ora to prevent Bingo ORA-28579 errors.
# Must run as startdb hook; standard Docker bind-mounting causes container crash.
set -euo pipefail

sqlnet_path=$(realpath "${TNS_ADMIN:-$ORACLE_BASE_HOME/network/admin}/sqlnet.ora")
cat > "$sqlnet_path" <<'EOF'
NAMES.DIRECTORY_PATH = (EZCONNECT, TNSNAMES)
EOF
echo "[bingo] Overrode $sqlnet_path (removed DISABLE_OOB=ON)"

# Drop cached sqlnet.ora state so new extproc agents read the new file.
lsnrctl reload >/dev/null
