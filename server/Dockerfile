from base:latest as build 

workdir /app
copy third-party ./third-party
copy CMakeLists.txt ./
copy server ./server

run mkdir build \
    && cd build \
    && cmake .. -GNinja -DCMAKE_BUILD_TYPE=Debug \
    && cmake --build . -j"$(nproc)"


from debian:trixie-slim as runtime

run apt-get update && apt-get install -y --no-install-recommends \
    libqt6core6 \
    libqt6network6 \
    libqt6websockets6 \
    libqt6sql6 \
    libqt6sql6-psql \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*
  
workdir /app
copy --from=build /app/build/server/server ./server
cmd ["./server"]


