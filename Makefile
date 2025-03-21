
.PHONY: run help build_dynamic_lib


run: ## Run the app
	@flutter run

run_main_c: ## Run the main.c file
	@echo "🔧 Running main.c file..."
	@./scripts/run_main_c.sh
	@echo "\n\n✅ main.c file executed successfully!"

build_libs: ## Build C language libraries
	@echo "🔧 Building C language libraries..."
	@make build_dynamic_lib
	@make build_darwin_lib
	@echo "✅ C language libraries built successfully!"

build_dynamic_lib: ## Build dynamic library
	@echo "🔧 Building dynamic library..."
	./scripts/build_dynamic_lib.sh
	@echo "✅ Dynamic library built successfully!"

build_darwin_lib: ## Build darwin library
	@echo "🔧 Building darwin library..."
	./scripts/build_darwin.sh
	@echo "✅ Darwin library built successfully!"

help:
	@echo "Available commands:"
	@grep -E '^[a-zA-Z_-]+:.*?## ' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'
