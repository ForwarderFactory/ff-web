#!/bin/sh

PREFIX="${PREFIX:-/usr/local}"
REPOSITORY="TheShadowEevee/Sharpii-NetCore"

LATEST_RELEASE="$(curl -s https://api.github.com/repos/${REPOSITORY}/releases/latest | grep '"tag_name":' | sed -E 's/.*"([^"]+)".*/\1/')"
if [ -z "$LATEST_RELEASE" ]; then
    echo "Failed to fetch the latest release version."
    exit 1
else
    echo "Latest release version: $LATEST_RELEASE"
fi

LINUX_ARM64="Sharpii-Net-Core-$LATEST_RELEASE-Linux-arm64.7z"
LINUX_AMD64="Sharpii-Net-Core-$LATEST_RELEASE-Linux-x64.7z"
MACOS_ARM64="Sharpii-Net-Core-$LATEST_RELEASE-MacOSX-arm64.7z"
MACOS_AMD64="Sharpii-Net-Core-$LATEST_RELEASE-MacOSX-x64.7z"

if [ "$(uname -s)" = "Linux" ]; then
    if [ "$(uname -m)" = "aarch64" ]; then
        FILENAME="$LINUX_ARM64"
    else
        FILENAME="$LINUX_AMD64"
    fi
elif [ "$(uname -s)" = "Darwin" ]; then
    if [ "$(uname -m)" = "arm64" ]; then
        FILENAME="$MACOS_ARM64"
    else
        FILENAME="$MACOS_AMD64"
    fi
else
    echo "Unsupported operating system."
    exit 1
fi

cd /tmp || exit 1

FILE="https://github.com/${REPOSITORY}/releases/download/${LATEST_RELEASE}/${FILENAME}"
echo "Downloading $FILENAME from $FILE"
curl -LOs "$FILE" || {
    echo "Failed to download $FILENAME"
    exit 1
}

echo "Extracting $FILENAME"
tar -xf "$FILENAME" || {
    echo "Failed to extract $FILENAME"
    exit 1
}

[ -f "Sharpii" ] && {
    echo "Sharpii binary found, moving to ${PREFIX}/bin"
    mv Sharpii ${PREFIX}/bin/Sharpii || {
        echo "Failed to move Sharpii binary to ${PREFIX}/bin"
        exit 1
    }
} || {
    echo "Sharpii binary not found after extraction."
    exit 1
}