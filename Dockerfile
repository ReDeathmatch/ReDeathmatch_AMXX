# syntax=docker/dockerfile:1

ARG MOD="cstrike"

FROM debian:trixie-slim AS build_stage

# Install required packages
RUN set -x \
    && apt-get update \
    && apt-get install -y --no-install-recommends --no-install-suggests \
       ca-certificates=20240203 \
       curl=8.5.0-2 \
       libarchive-tools=3.7.2-1 \
       lib32stdc++6=14-20240201-3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/local/bin/
ADD --chmod=755 https://raw.githubusercontent.com/hldsdocker/rehlds/master/utils/GetGithubReleaseUrl.sh GetGithubReleaseUrl

WORKDIR /root/hlds/cstrike 

# Install Metamod-R
RUN releaseLink="https://github.com/theAsmodai/metamod-r/releases/download/1.3.0.149/metamod-bin-1.3.0.149.zip" \
    && curl -sSL ${releaseLink} | bsdtar -xf - --exclude='*.dll' --exclude='*.pdb' addons/*

# Install AMXModX 1.9.0
ARG AMXModX_URL="https://www.amxmodx.org/amxxdrop/1.9/amxmodx-1.9.0-git5294-base-linux.tar.gz"
RUN curl -sSL ${AMXModX_URL} | bsdtar -xf - addons/  \
    && echo "linux addons/amxmodx/dlls/amxmodx_mm_i386.so" > addons/metamod/plugins.ini

# Install ReAPI
RUN releaseLink="https://github.com/s1lentq/reapi/releases/download/5.24.0.300/reapi-bin-5.24.0.300.zip" \
    && curl -sSL ${releaseLink} | bsdtar -xf - --exclude='*.dll' --exclude='*.pdb' addons/

COPY cstrike .

SHELL ["/bin/bash", "-c"]

WORKDIR /usr/local/bin/
COPY --chmod=755 .vscode/build.sh BuildAMXXPlugins

WORKDIR /root/hlds/cstrike/addons/amxmodx/
RUN BuildAMXXPlugins . .


WORKDIR /root/hlds/cstrike/
ARG YaPB_URL="https://github.com/yapb/yapb/releases/download/4.4.957/yapb-4.4.957-linux.tar.xz"
RUN curl -sSL ${YaPB_URL} | bsdtar -xf - addons/ \
    && echo "linux addons/yapb/bin/yapb.so" >> addons/metamod/plugins.ini


ARG MOD
FROM hldsdocker/rehlds-${MOD}:regamedll AS run_stage

COPY --chown=${APPUSER}:${APPUSER} --chmod=755 --from=build_stage /root/hlds/cstrike ${MOD}

# Activate Metamod-R
RUN sed -i 's/gamedll_linux ".*"/gamedll_linux "addons\/metamod\/metamod_i386.so"/' ${MOD}/liblist.gam
