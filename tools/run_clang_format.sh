#!/bin/bash

# Path to clang-format executable
CLANG_FORMAT="clang-format"

# Check if clang-format is available
if ! command -v "$CLANG_FORMAT" &> /dev/null; then
    echo "Error: clang-format not found. Please ensure it's installed and in your PATH."
    exit 1
fi

# Run clang-format on .cpp and .h files in root/ folder (non-recursive)
#  Best to exclude MTISerial.cpp and enumCOMs.cpp to prevent porting issues 
#  from our master C++ codebase
find ./ -maxdepth 1 -type f \( -name "*.cpp" -o -name "*.h" \) \
    -not -name "MTISerial.cpp" \
    -not -name "enumCOMs.cpp" \
    -print0 | xargs -0 "$CLANG_FORMAT" -i -style=file

# Format demo_source/ files (non-recursive)
find ./demo_source -maxdepth 1 -type f \( -name "*.cpp" -o -name "*.h" \) -print0 | xargs -0 "$CLANG_FORMAT" -i -style=file

# Format include/ files (non-recursive)
find ./include -maxdepth 1 -type f \( -name "*.cpp" -o -name "*.h" \) -print0 | xargs -0 "$CLANG_FORMAT" -i -style=file

echo "Formatting complete."