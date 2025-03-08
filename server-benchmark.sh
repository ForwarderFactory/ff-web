#!/bin/sh

PORT=8080
[ ! -z "$1" ] && PORT=$1

# benchmark the server, only the root endpoint
ab -n 1000 -c 10 http://localhost:$PORT/