FROM ghcr.io/dragonminded/libdragon:latest

ENV DOCKER_CONTAINER=true

ARG USERNAME=dev
ARG USER_UID=1000
ARG USER_GID=$USER_UID

# Create dev user for rootless access
RUN groupadd --gid $USER_GID $USERNAME \
 && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME --shell /bin/bash

# Install libdragon SDK (preview branch)
# Preview branch includes: OpenGL 1.1, MPEG1 video, TrueType fonts, multithreading
# See: https://github.com/DragonMinded/libdragon/wiki/Preview-branch
RUN cd /tmp \
 && git clone --branch preview --depth 1 https://github.com/DragonMinded/libdragon.git \
 && cd libdragon \
 && ./build.sh \
 && rm -rf /tmp/libdragon

# This is the default location for the toolchain in the upstream image. Should be owned by the dev user
RUN chown -R $USER_UID:$USER_GID /n64_toolchain

USER dev

WORKDIR /workspaces/n64-tech-demo
