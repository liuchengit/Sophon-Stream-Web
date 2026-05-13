#!/bin/bash
set -e

echo "Deploying Sophon-Stream Web Management System..."

INSTALL_DIR="${1:-/opt/sophon-stream-web}"

echo "Installing to: $INSTALL_DIR"

# Create installation directory
mkdir -p "$INSTALL_DIR"
mkdir -p "$INSTALL_DIR/bin"
mkdir -p "$INSTALL_DIR/www"
mkdir -p "$INSTALL_DIR/data"
mkdir -p "$INSTALL_DIR/logs"
mkdir -p "$INSTALL_DIR/configs"
mkdir -p "$INSTALL_DIR/models"

# Copy backend binary
if [ -f "./build/backend/sophon-stream-web" ]; then
    cp ./build/backend/sophon-stream-web "$INSTALL_DIR/bin/"
    echo "Backend binary installed"
else
    echo "Error: Backend binary not found. Run build.sh first."
    exit 1
fi

# Copy frontend build
if [ -d "./backend/www" ]; then
    cp -r ./backend/www/* "$INSTALL_DIR/www/"
    echo "Frontend files installed"
else
    echo "Warning: Frontend build not found"
fi

# Copy config files
cp ./backend/config.json "$INSTALL_DIR/configs/" 2>/dev/null || true
cp ./engine/configs/*.json "$INSTALL_DIR/configs/" 2>/dev/null || true
echo "Config files installed"

# Copy scripts
cp ./scripts/start.sh "$INSTALL_DIR/bin/"
chmod +x "$INSTALL_DIR/bin/start.sh"
echo "Startup script installed"

# Set permissions
chmod +x "$INSTALL_DIR/bin/sophon-stream-web"

echo ""
echo "Deployment completed!"
echo "Installation directory: $INSTALL_DIR"
echo ""
echo "To start the service:"
echo "  cd $INSTALL_DIR && ./bin/start.sh"
echo ""
echo "Default admin credentials:"
echo "  Username: admin"
echo "  Password: admin123"
