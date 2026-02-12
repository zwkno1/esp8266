#!/bin/bash

NGX_DIR=${NGX_DIR:-/usr/local/openresty/nginx}

HTML_DIR=${NGX_DIR}/html
LUA_DIR=${NGX_DIR}/lua

mkdir -p ${HTML_DIR}
mkdir -p ${LUA_DIR}

cp -r html/* ${HTML_DIR}
cp -r lua/* ${LUA_DIR}

