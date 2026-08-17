ARG UBUNTU_VERSION=24.04
FROM ubuntu:${UBUNTU_VERSION}

ARG DEBIAN_FRONTEND=noninteractive

ENV LANG=C.UTF-8 \
    LC_ALL=C.UTF-8 \
    TZ=UTC

# Host dependencies recommended by Buildroot plus the tools needed for
# ARM64 kernel/device-tree/U-Boot image work.
RUN apt-get update \
    && apt-get install --no-install-recommends --yes \
        bc \
        bison \
        build-essential \
        bzip2 \
        ca-certificates \
        ccache \
        cpio \
        device-tree-compiler \
        dosfstools \
        dwarves \
        e2fsprogs \
        fakeroot \
        file \
        flex \
        gawk \
        git \
        gosu \
        gzip \
        libelf-dev \
        libncurses-dev \
        libssl-dev \
        lz4 \
        make \
        mercurial \
        mtools \
        patch \
        perl \
        pkg-config \
        python3 \
        python3-setuptools \
        rsync \
        socat \
        tar \
        unzip \
        u-boot-tools \
        wget \
        xxd \
        which \
        xz-utils \
        zstd \
    && rm -rf /var/lib/apt/lists/*

COPY docker/entrypoint.sh /usr/local/bin/buildroot-entrypoint
RUN chmod 0755 /usr/local/bin/buildroot-entrypoint

WORKDIR /workspace
ENTRYPOINT ["/usr/local/bin/buildroot-entrypoint"]
CMD ["bash"]
