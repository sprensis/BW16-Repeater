#!/usr/bin/env bash
set -euo pipefail
PORT=${PORT:-/dev/tty.usbserial-0001}
FQBN=${FQBN:-realtek:AmebaD:Ai-Thinker_BW16}
if ! arduino-cli board listall | awk 'NR>1{print $2}' | grep -qx "$FQBN"; then
  echo "Invalid FQBN: $FQBN" >&2
  echo "Available AmebaD boards:" >&2
  arduino-cli board listall | awk 'NR>1 && /realtek:AmebaD/{printf("- %s\n", $2)}'
  exit 1
fi
arduino-cli upload --port "$PORT" --fqbn "$FQBN" ./ || {
  echo "Upload failed. Verify FQBN. Available AmebaD boards:" >&2
  arduino-cli board listall | sed -n '1p;/realtek:AmebaD/p'
  exit 1
}