#!/bin/bash

if [[ "${BASH_SOURCE[0]}" != "${0}" ]]; then
  SOURCED=true
else
  SOURCED=false
fi

if [[ "$SOURCED" == "false" ]]; then
  echo "This script must be sourced, not executed!" >&2
  echo "Please rerun with 'source register.sh'" >&2
  exit 1
fi

echo "We're setting up an alias to the pico.sh script in your shell profile."
echo "---"
echo "\t $ alias pico="$PWD/scripts/pico.sh""
alias pico="$PWD/scripts/pico.sh"
echo "---"
echo "You can now call \`pico\` directly."
echo
echo "Try running: \`pico compile\` to see it work."