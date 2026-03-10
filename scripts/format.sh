#!/bin/bash
# Format C++ files using clang-format

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "=== C++ Formatter ==="
echo "Project root: $PROJECT_ROOT"
echo ""

# Find all C++ files
CPP_FILES=$(find "$PROJECT_ROOT/include" "$PROJECT_ROOT/src" -type f \( -name "*.h" -o -name "*.hpp" -o -name "*.cpp" -o -name "*.cc" -o -name "*.cxx" \) 2>/dev/null || true)

if [ -z "$CPP_FILES" ]; then
    echo "No C++ files found."
    exit 0
fi

# Count files
FILE_COUNT=$(echo "$CPP_FILES" | wc -l)
echo "Found $FILE_COUNT C++ files"
echo ""

# Check if clang-format is installed
if ! command -v clang-format &> /dev/null; then
    echo "Error: clang-format is not installed"
    echo "Install: sudo apt-get install clang-format"
    exit 1
fi

echo "Formatting files..."
echo "$CPP_FILES" | xargs clang-format -i

echo ""
echo "✓ Formatting completed successfully"
