#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

# Store the repository root directory
REPO_ROOT="$(pwd)"
echo "Repository root: ${REPO_ROOT}"

# Determine build configuration (default to Release for CD workflow)
# First argument can be "Debug" or "Release"
BUILD_TYPE=${1:-Release}
echo "Starting build process with configuration: ${BUILD_TYPE}"

# Clean and create build directory for the specified configuration
BUILD_DIR="${REPO_ROOT}/${BUILD_TYPE}"
rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}

# Build the project with the specified configuration
cd ${BUILD_DIR}
cmake -DCMAKE_BUILD_TYPE=${BUILD_TYPE} ..
echo "Building project in ${BUILD_TYPE} mode..."
make

# Create delivery directory at repository root
echo "Creating delivery directory at repository root..."
DELIVERY_DIR="${REPO_ROOT}/delivery"
rm -rf ${DELIVERY_DIR}
mkdir -p ${DELIVERY_DIR}

# Find and copy build artifacts
echo "Copying build artifacts from ${BUILD_TYPE} to delivery directory..."

# Copy the shared library if it exists
if [ -f "${BUILD_DIR}/libPlayzerX.so" ]; then
    cp ${BUILD_DIR}/libPlayzerX.so ${DELIVERY_DIR}/
    echo "Copied libPlayzerX.so"
else
    echo "Warning: libPlayzerX.so not found"
    # List contents to help debug
    echo "Contents of ${BUILD_TYPE} directory:"
    ls -la ${BUILD_DIR}/
fi

# Copy the demo executable if it exists
if [ -d "${BUILD_DIR}/demo_source" ] && [ -f "${BUILD_DIR}/demo_source/PlayzerX-Demo" ]; then
    cp ${BUILD_DIR}/demo_source/PlayzerX-Demo ${DELIVERY_DIR}/
    echo "Copied PlayzerX-Demo"
else
    echo "Warning: PlayzerX-Demo not found"
    # Try to find it elsewhere
    DEMO_PATH=$(find ${BUILD_DIR} -name "PlayzerX-Demo" -type f)
    if [ ! -z "$DEMO_PATH" ]; then
        cp $DEMO_PATH ${DELIVERY_DIR}/
        echo "Found and copied PlayzerX-Demo from $DEMO_PATH"
    else
        echo "Could not find PlayzerX-Demo executable"
        # List contents to help debug
        if [ -d "${BUILD_DIR}/demo_source" ]; then
            echo "Contents of ${BUILD_DIR}/demo_source directory:"
            ls -la ${BUILD_DIR}/demo_source/
        fi
    fi
fi

# Print directory structure for verification
echo "Contents of delivery directory:"
ls -la ${DELIVERY_DIR}/

# Check if delivery directory contains files
if [ -z "$(ls -A ${DELIVERY_DIR}/)" ]; then
    echo "Warning: delivery directory is empty. Build may have failed to produce artifacts."
    echo "Listing contents of build directory for debugging:"
    find ${BUILD_DIR} -type f -name "*.so" -o -name "PlayzerX-Demo" | sort
    exit 1
else
    echo "Build completed successfully. Delivery directory is at: ${DELIVERY_DIR}"
    # Return to the repository root to ensure the workflow can find the delivery directory
    cd ${REPO_ROOT}
fi

# Exit with status 0
exit 0
