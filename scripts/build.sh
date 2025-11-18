#!/usr/bin/env bash
set -euo pipefail
FQBN=${FQBN:-realtek:AmebaD:Ai-Thinker_BW16}
arduino-cli core update-index
arduino-cli core install realtek:AmebaD || true
arduino-cli compile --fqbn "$FQBN" ./ || {
  echo "Build failed. Verify FQBN. Available AmebaD boards:" >&2
  arduino-cli board listall | sed -n '1p;/realtek:AmebaD/p'
  exit 1
}