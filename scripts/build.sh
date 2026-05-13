#!/bin/bash
set -e

echo "Building Sophon-Stream Web Management System..."

# Build frontend
echo "Building frontend..."
cd frontend
npm install
npm run build
cd ..

# Build backend
echo "Building backend..."
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
cd ..

echo "Build completed successfully!"
echo "Run with: ./build/backend/sophon-stream-web"
