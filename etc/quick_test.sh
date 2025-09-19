#!/bin/sh
LUA_DIR="./"
make -C "$LUA_DIR"
gcc -Wall -O3 -I"$LUA_DIR" -o "$LUA_DIR/etc/lua_issequence_test" "$LUA_DIR/etc/lua_issequence_test.c" -L"$LUA_DIR" -l:liblua.a -lm
echo "Running tests / benchmarks"
"$LUA_DIR/etc/lua_issequence_test"
