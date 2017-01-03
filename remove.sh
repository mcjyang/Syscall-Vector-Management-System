#!/bin/sh
set -x
# WARNING: this script doesn't check for errors, so you have to enhance it in case any of the commands
# below fail.

rmmod vector_2
rmmod vector_1
rmmod sv_manage
make clean