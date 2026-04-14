#!/bin/bash
# Performance benchmark: rloc vs cloc
# Usage: tools/benchmark.sh <file_or_directory>

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

PATH_ARG="$1"
if [ -z "$PATH_ARG" ]; then
    echo "Usage: $0 <file_or_directory>"
    exit 1
fi

echo "=== Benchmark: rloc vs cloc ==="
echo "Target: $PATH_ARG"
echo ""

# Find rloc binary - check project dir first, then PATH
if [ -f "$PROJECT_DIR/rloc" ]; then
    RLOC_BIN="$PROJECT_DIR/rloc"
elif command -v rloc &> /dev/null; then
    RLOC_BIN="rloc"
else
    echo "Error: rloc binary not found in $PROJECT_DIR or PATH"
    exit 1
fi

# Measure rloc
echo "rloc timing:"
if command -v gtime &> /dev/null; then
    # Use gtime (GNU time) on macOS if available
    gtime -f "Elapsed: %E seconds (User: %U, System: %S)" "$RLOC_BIN" "$PATH_ARG" 2>&1 | tail -10
elif /usr/bin/time -f "Elapsed: %E" true 2>/dev/null; then
    # GNU time format supported
    /usr/bin/time -f "Elapsed: %E seconds (User: %U, System: %S)" "$RLOC_BIN" "$PATH_ARG" 2>&1 | tail -10
else
    # BSD time (macOS default) - less precise
    echo "Timing with BSD time (less precise):"
    time "$RLOC_BIN" "$PATH_ARG" 2>&1
fi
echo ""

# Measure cloc (if available)
if command -v cloc &> /dev/null; then
    echo "cloc timing:"
    if command -v gtime &> /dev/null; then
        gtime -f "Elapsed: %E seconds (User: %U, System: %S)" cloc "$PATH_ARG" 2>&1 | tail -10
    elif /usr/bin/time -f "Elapsed: %E" true 2>/dev/null; then
        /usr/bin/time -f "Elapsed: %E seconds (User: %U, System: %S)" cloc "$PATH_ARG" 2>&1 | tail -10
    else
        echo "Timing with BSD time (less precise):"
        time cloc "$PATH_ARG" 2>&1
    fi
else
    echo "cloc not installed - skipping cloc benchmark"
fi
