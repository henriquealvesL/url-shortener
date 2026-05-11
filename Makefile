CMAKE ?= cmake

SERVER_BUILD_DIR := build/server-build
PROXY_BUILD_DIR := build/proxy-build
DEPS_DIR := $(CURDIR)/build/_deps
SERVER_BIN := $(SERVER_BUILD_DIR)/server
PROXY_BIN := $(PROXY_BUILD_DIR)/proxy

.PHONY: all build server proxy run run-server run-proxy demo-py stop-server stop-proxy stop clean

all: build

build: server proxy

server:
	$(CMAKE) -S server -B $(SERVER_BUILD_DIR) -DFETCHCONTENT_BASE_DIR=$(DEPS_DIR)
	$(CMAKE) --build $(SERVER_BUILD_DIR)

proxy:
	$(CMAKE) -S proxy -B $(PROXY_BUILD_DIR) -DFETCHCONTENT_BASE_DIR=$(DEPS_DIR)
	$(CMAKE) --build $(PROXY_BUILD_DIR)

run: build
	@echo "Subindo servidor REST e proxy..."
	@echo "REST:  http://127.0.0.1:8080"
	@echo "Proxy: tcp://127.0.0.1:9000"
	@echo "Pressione Ctrl+C para encerrar."
	@$(SERVER_BIN) & \
	server_pid=$$!; \
	trap 'kill $$server_pid 2>/dev/null || true' INT TERM EXIT; \
	sleep 1; \
	$(PROXY_BIN)

run-server: server
	$(SERVER_BIN)

run-proxy: proxy
	$(PROXY_BIN)

demo-py:
	python3 client_py/example.py

stop-server:
	@lsof -tiTCP:8080 -sTCP:LISTEN | xargs -r kill

stop-proxy:
	@lsof -tiTCP:9000 -sTCP:LISTEN | xargs -r kill

stop:
	@$(MAKE) stop-server
	@$(MAKE) stop-proxy

clean:
	$(CMAKE) -E remove_directory build
