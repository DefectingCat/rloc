#!/bin/bash
# Compare rloc output against cloc for a single file or directory
# Usage: tools/compare_cloc.sh <file_or_dir>
# Requires: cloc installed

FILE="$1"
if [ -z "$FILE" ]; then
    echo "Usage: $0 <file_or_dir>"
    exit 1
fi

echo "=== rloc output ==="
./rloc "$FILE"
echo ""
echo "=== cloc output ==="
cloc "$FILE"