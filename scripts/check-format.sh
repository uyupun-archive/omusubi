#!/bin/bash
# Check if C++ files are properly formatted (for CI/CD)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "=== C++ Format Check ==="
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

echo "Checking format..."
echo ""

UNFORMATTED_FILES=""

# Check each file
while IFS= read -r file; do
    # Check if file needs formatting
    if ! clang-format --dry-run -Werror "$file" 2>&1 | grep -q "no modified files"; then
        # Get diff
        DIFF=$(clang-format "$file" | diff -u "$file" - || true)
        if [ -n "$DIFF" ]; then
            UNFORMATTED_FILES="$UNFORMATTED_FILES$file\n"
            echo "✗ $file needs formatting"
        fi
    fi
done <<< "$CPP_FILES"

echo ""

if [ -n "$UNFORMATTED_FILES" ]; then
    echo "=== Unformatted Files ==="
    echo -e "$UNFORMATTED_FILES"
    echo ""
    echo "✗ Format check failed"
    echo "Run: ./scripts/format.sh"
    exit 1
else
    echo "✓ All files are properly formatted"
fi
