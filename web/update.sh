#!/bin/bash
set -e

cd ~/esp8266

hash1=$(git rev-parse HEAD)
git fetch origin
hash2=$(git rev-parse @{u})

if [ "$hash1" = "$hash2" ]; then
    echo "skip"
    exit 0
fi

echo "git updated"

git pull

bash ./web/install.sh

nginx -s reload

echo "ok"