PACKAGE_NAME := pymaeparser
PACKAGE_DIR  := src/pymaeparser

CONDA_ENV_RUN = conda run --no-capture-output --name $(PACKAGE_NAME)

.PHONY: env build lint format test

env:
	mamba create     --name $(PACKAGE_NAME)
	mamba env update --name $(PACKAGE_NAME) --file environment.yml
	$(CONDA_ENV_RUN) pip install --no-build-isolation -e .
	$(CONDA_ENV_RUN) pre-commit install || true

build:
	$(CONDA_ENV_RUN) pip install --no-build-isolation -ve .

lint:
	$(CONDA_ENV_RUN) ruff check $(PACKAGE_DIR)

format:
	$(CONDA_ENV_RUN) ruff format $(PACKAGE_DIR)
	$(CONDA_ENV_RUN) ruff check --fix --select I $(PACKAGE_DIR)

test:
	$(CONDA_ENV_RUN) pytest -v --color=yes tests/
