#!/bin/sh

rm -rf ff-web
git clone --recursive https://github.com/ForwarderFactory/ff-web; cd ff-web || exit 1
cp -r css/ /etc/ff/
cp -r js/ /etc/ff/
cp -r html/ /etc/ff/

git clone https://github.com/ForwarderFactory/ff-web-assets .assets/
cp -r .assets/* /etc/ff/
rm -rf .assets/

[ ! -f "/etc/ff/config.yaml" ] && ff-web -gc > /etc/ff/config.yaml

chown -R ff-web:ff-web /etc/ff /var/log/ff /var/db/ff /var/lib/ff
chmod -R 755 /etc/ff /var/log/ff /var/db/ff /var/lib/ff

systemctl daemon-reload
systemctl enable ff-web
systemctl start ff-web
