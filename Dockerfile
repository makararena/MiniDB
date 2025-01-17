# Here S30039 gave me an idea of creating Dockerfile for this app
# Stage 1: Build
FROM ubuntu:22.04 AS builder

# Install build dependencies including fmt
RUN apt-get update && \
    apt-get install -y \
        cmake \
        g++ \
        make \
        libfmt-dev \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Copy the source code into the container
COPY . /app

# Build the project
RUN cmake -B build -S . && cmake --build build --target SimpleDatabase

# Stage 2: Runtime
FROM ubuntu:22.04

# Install runtime dependencies (use libfmt-dev to ensure compatibility)
RUN apt-get update && \
    apt-get install -y \
        libfmt-dev \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /app

# Copy the compiled binary from the builder stage
COPY --from=builder /app/build/SimpleDatabase /usr/local/bin/SimpleDatabase

# Set the default command to run the application
CMD ["/usr/local/bin/SimpleDatabase"]
