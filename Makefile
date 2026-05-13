.PHONY: all clean build frontend backend install run dev test

CMAKE_BUILD_TYPE ?= Release
BUILD_DIR ?= build

all: build

build: frontend backend

frontend:
	cd frontend && npm install && npm run build

backend:
	mkdir -p $(BUILD_DIR) && cd $(BUILD_DIR) && \
	cmake .. -DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) && \
	make -j$$(nproc)

clean:
	rm -rf $(BUILD_DIR)
	rm -rf backend/www/*
	rm -rf frontend/node_modules

install: build
	cd $(BUILD_DIR) && make install

run:
	./$(BUILD_DIR)/backend/sophon-stream-web

dev:
	cd frontend && npm run dev

test:
	cd frontend && npm run test
	cd $(BUILD_DIR) && ctest
