#!/bin/bash

set -e

dir=$(dirname $(readlink -f "$0"))

NGX_DIR=${NGX_DIR:-/usr/local/openresty/nginx}

HTML_DIR=${NGX_DIR}/html
LUA_DIR=${NGX_DIR}/lua

mkdir -p ${HTML_DIR}
mkdir -p ${LUA_DIR}

cp -rf ${dir}/html/* ${HTML_DIR}
cp -rf ${dir}/lua/* ${LUA_DIR}

echo "updated!"