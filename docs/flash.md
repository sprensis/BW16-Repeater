# Flashing the Device

- Connect BW16 via USB‑UART
- Identify the port (`/dev/tty.usbserial-*`)
- `PORT=/dev/tty.usbserial-0001 FQBN=realtek:AmebaD:Ai-Thinker_BW16 bash scripts/flash.sh`