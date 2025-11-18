# Build on macOS

- Install `brew install arduino-cli`
- Initialize Arduino CLI if needed:
  - `arduino-cli config init`
  - `arduino-cli config set board_manager.additional_urls https://downloads.arduino.cc/packages/package_index.json`
- Update index: `arduino-cli core update-index`
- Install AmebaD core: `arduino-cli core install realtek:AmebaD`
- Build: `FQBN=realtek:AmebaD:Ai-Thinker_BW16 bash scripts/build.sh`
- List boards: `arduino-cli board listall | grep -i AmebaD`