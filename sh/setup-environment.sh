#!/bin/sh

if ! command -v apt-get >/dev/null 2>&1; then
    echo "apt-get not found, you must deploy this on an Ubuntu/Debian system"
    exit 1
fi

apt-get update && apt-get install -y \
    cmake \
    g++ \
    make \
    git \
    libboost-all-dev \
    libssl-dev \
    libyaml-cpp-dev \
    libpq-dev \
    libsqlite3-dev \
    nlohmann-json3-dev \
    libxml2-dev \
    libc6-dev \
    nodejs \
    postgresql \
    sqlite3 \
    ffmpeg \
    imagemagick \
    libmagick++-dev \
    libavcodec-dev \
    libavformat-dev \
    libavdevice-dev \
    libpostproc-dev \
    libavutil-dev \
    libswscale-dev \
    npm || exit 1

npm install -g uglify-js

groupadd -r ff-web && useradd -r -g ff-web ff-web
mkdir -p /etc/ff /var/log/ff /var/db/ff /var/lib/ff

rm -rf ff-web
git clone --recursive https://github.com/ForwarderFactory/ff-web; cd ff-web || exit 1
mkdir -p build && cd build || exit 1
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
make && make install || exit 1
cd ..; rm -rf build
cp -r css/ /etc/ff/
cp -r js/ /etc/ff/
cp -r html/ /etc/ff/

git clone https://github.com/ForwarderFactory/ff-web-assets .assets/
cp -r .assets/* /etc/ff/
rm -rf .assets/

[ ! -f "/etc/ff/config.yaml" ] && ff-web -gc > /etc/ff/config.yaml

chown -R ff-web:ff-web /etc/ff /var/log/ff /var/db/ff /var/lib/ff
chmod -R 755 /etc/ff /var/log/ff /var/db/ff /var/lib/ff

cat > /etc/systemd/system/ff-web.service <<EOF
[Unit]
Description=ff-web
After=network.target

[Service]
Type=simple
User=ff-web
Group=ff-web
ExecStart=ff-web
Restart=on-failure

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl enable ff-web
systemctl restart ff-web