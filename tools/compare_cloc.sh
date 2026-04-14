#!/bin/bash
# Compare rloc output against cloc for a single file or directory
# Usage: tools/compare_cloc.sh <file_or_dir>

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

FILE="$1"
if [ -z "$FILE" ]; then
    echo "Usage: $0 <file_or_dir>"
    exit 1
fi

# Find rloc binary - check project dir first, then PATH
if [ -f "$PROJECT_DIR/rloc" ]; then
    RLOC_BIN="$PROJECT_DIR/rloc"
elif command -v rloc &> /dev/null; then
    RLOC_BIN="rloc"
else
    echo "Error: rloc binary not found in $PROJECT_DIR or PATH"
    exit 1
fi

# Check if cloc is installed
if ! command -v cloc >/dev/null 2>&1; then
    echo "Warning: cloc is not installed. Install it for comparison."
    echo "         Only rloc output will be shown."
    echo ""
fi

if [ -f "$FILE" ]; then
    # Single file comparison
    echo "=== Single file comparison ==="
    echo "File: $FILE"
    echo ""
    echo "rloc output:"
    "$RLOC_BIN" "$FILE" 2>/dev/null
    echo ""
    if command -v cloc >/dev/null 2>&1; then
        echo "cloc output:"
        cloc "$FILE"
    fi
elif [ -d "$FILE" ]; then
    # Directory comparison
    echo "=== Directory comparison ==="
    echo "Directory: $FILE"
    echo ""
    echo "rloc output:"
    "$RLOC_BIN" "$FILE" 2>/dev/null
    echo ""
    if command -v cloc >/dev/null 2>&1; then
        echo "cloc output:"
        cloc "$FILE"
    fi
else
    echo "Error: '$FILE' is not a valid file or directory"
    exit 1
fi