#!/bin/bash
# Pre-commit hook: Format and lint staged C++ files

set -e

echo "=== Pre-commit Hook ==="
echo ""

# Get the project root
PROJECT_ROOT="$(git rev-parse --show-toplevel)"

# Get staged C++ files
STAGED_FILES=$(git diff --cached --name-only --diff-filter=ACM | grep -E '\.(h|hpp|cpp|cc|cxx)$' || true)

if [ -z "$STAGED_FILES" ]; then
    echo "No C++ files staged for commit."
    exit 0
fi

echo "Staged C++ files:"
echo "$STAGED_FILES" | sed 's/^/  - /'
echo ""

# Format staged files
echo "1. Formatting files..."
for file in $STAGED_FILES; do
    if [ -f "$file" ]; then
        clang-format -i "$file"
        git add "$file"
    fi
done
echo "   ✓ Formatting completed"
echo ""

# Run lint on staged files
echo "2. Running lint..."
ERROR_COUNT=0

for file in $STAGED_FILES; do
    if [ -f "$file" ]; then
        # Skip platform-specific implementation files (require hardware-specific headers)
        if [[ "$file" == *"/platform/"*".cpp" ]]; then
            echo "   ⊘ Skipping platform-specific file: $file"
            continue
        fi

        # Determine language flag for .h files
        LANG_FLAG=""
        if [[ "$file" == *.h ]]; then
            LANG_FLAG="-x c++"
        fi

        # Run clang-tidy and capture errors
        OUTPUT=$(clang-tidy "$file" -- -Iinclude -std=c++17 $LANG_FLAG 2>&1 || true)
        FILE_ERRORS=$(echo "$OUTPUT" | grep -c "error:" || true)

        if [ $FILE_ERRORS -gt 0 ]; then
            echo "   ✗ $file has $FILE_ERRORS error(s)"
            echo "$OUTPUT" | grep "error:" | head -5
            ERROR_COUNT=$((ERROR_COUNT + FILE_ERRORS))
        fi
    fi
done

if [ $ERROR_COUNT -gt 0 ]; then
    echo ""
    echo "✗ Commit blocked: $ERROR_COUNT lint error(s) found"
    echo "  Fix errors and try again, or use 'git commit --no-verify' to bypass"
    exit 1
fi

echo "   ✓ Lint passed"
echo ""
echo "✓ Pre-commit checks passed"
