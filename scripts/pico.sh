#!/bin/bash
set -e

# Move to the root of the Git repository
cd "$(git rev-parse --show-toplevel)"

# Check for argument
if [[ $# -lt 1 ]]; then
  echo "Usage: $0 {compile|clean|logs|upload|run} [device]" >&2
  exit 1
fi

INPUT_COMMAND="$1"
DEVICE_NAME="${2:-picocalc.local}"  # Default device if not specified

# Activate virtual environment if not already active
if [[ -z "$VIRTUAL_ENV" ]]; then
  source ./esphome/venv/bin/activate
fi

# Expand 'run' to multiple commands
if [[ "$INPUT_COMMAND" == "run" ]]; then
  COMMANDS=("compile" "upload" "logs")
else
  COMMANDS=("$INPUT_COMMAND")
fi

# Iterate through each command
for COMMAND in "${COMMANDS[@]}"; do
  DEVICE_PARAM=""
  if [[ "$COMMAND" == "logs" || "$COMMAND" == "upload" ]]; then
    DEVICE_PARAM="--device $DEVICE_NAME"
  fi

  echo "Running: python -m esphome -v $COMMAND picocalc.yaml $DEVICE_PARAM"
  python -m esphome -v "$COMMAND" picocalc.yaml $DEVICE_PARAM
  sleep 1  # Add a short delay between commands
done
