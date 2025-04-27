#!/bin/bash
set -e
# Move to the root of the Git repository
cd "$(git rev-parse --show-toplevel)"

./esphome/scripts/setup.sh

