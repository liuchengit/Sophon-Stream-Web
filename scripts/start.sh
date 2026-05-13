#!/bin/bash
set -e

echo "Starting Sophon-Stream Web Management System..."

# Check if backend binary exists
if [ ! -f "./build/backend/sophon-stream-web" ]; then
    echo "Error: Backend binary not found. Run build.sh first."
    exit 1
fi

# Start backend
cd build/backend
./sophon-stream-web
