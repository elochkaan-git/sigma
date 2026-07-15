.PHONY: build up

build:
	docker compose build base
	docker compose build server

up: build
	docker compose up

down:
	docker compose down -v