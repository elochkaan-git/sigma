from debian:trixie-slim as build
run apt-get update && apt-get install -y --no-install-recommends \
  build-essential \
  cmake \
  ninja-build \
  pkg-config \
  ca-certificates \
  qt6-base-dev \
  qt6-websockets-dev \
  libqt6sql6-psql \
  && rm -rf /var/lib/apt/lists/*