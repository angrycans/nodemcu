#!/usr/bin/env bash
set -euo pipefail

HOST="root@45.32.106.210"
REMOTE_DIR="/var/www/servo"

LOCAL_DIR="$(cd "$(dirname "$0")/.." && pwd)/web/servo"

rsync -avz --delete \
  --exclude '.DS_Store' \
  "${LOCAL_DIR}/" \
  "${HOST}:${REMOTE_DIR}/"

echo "Deploy complete: ${HOST}:${REMOTE_DIR}"
