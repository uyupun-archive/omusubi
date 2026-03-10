#!/bin/bash
# Lint C++ files using clang-tidy

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "=== C++ Linter ==="
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

# Check if clang-tidy is installed
if ! command -v clang-tidy &> /dev/null; then
    echo "Error: clang-tidy is not installed"
    echo "Install: sudo apt-get install clang-tidy"
    exit 1
fi

echo "Running clang-tidy..."
echo ""

ERROR_COUNT=0
WARNING_COUNT=0

# Process each file
while IFS= read -r file; do
    # Skip platform-specific implementation files (require hardware-specific headers)
    if [[ "$file" == *"/platform/"*".cpp" ]]; then
        continue
    fi

    echo "Checking: $file"

    # Determine language flag for .h files
    LANG_FLAG=""
    if [[ "$file" == *.h ]]; then
        LANG_FLAG="-x c++"
    fi

    # Run clang-tidy and capture output
    OUTPUT=$(clang-tidy "$file" -- -Iinclude -std=c++17 $LANG_FLAG 2>&1 || true)

    # Count errors and warnings in this file
    FILE_ERRORS=$(echo "$OUTPUT" | grep -c "error:" || true)
    FILE_WARNINGS=$(echo "$OUTPUT" | grep -c "warning:" || true)

    ERROR_COUNT=$((ERROR_COUNT + FILE_ERRORS))
    WARNING_COUNT=$((WARNING_COUNT + FILE_WARNINGS))

    # Show only project file warnings (filter out system headers)
    echo "$OUTPUT" | grep "^$PROJECT_ROOT" || true

done <<< "$CPP_FILES"

echo ""
echo "=== Summary ==="
echo "Errors: $ERROR_COUNT"
echo "Warnings: $WARNING_COUNT"
echo ""

if [ $ERROR_COUNT -gt 0 ]; then
    echo "✗ Linting failed with $ERROR_COUNT error(s)"
    exit 1
else
    echo "✓ Linting completed successfully"
fi
