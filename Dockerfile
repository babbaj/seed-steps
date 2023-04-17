# Set the base image to Ubuntu 20.04
FROM ubuntu:22.04

# Update the package manager
RUN apt-get update

# Install necessary build tools
RUN apt-get install -y cmake clang pkg-config

# Install fmtlib
RUN apt-get install -y libfmt-dev

# Copy the source code into the container
COPY . /app

# Set the working directory to the app directory
WORKDIR /app

# Build the application with clang 15
RUN mkdir build && \
    cd build && \
    CC=clang CXX=clang++ cmake .. && \
    make -j`nproc`

# Set the entrypoint to the application
ENTRYPOINT ["/app/build/mansion_seeds"]