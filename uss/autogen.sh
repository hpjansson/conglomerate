#!/bin/sh
PS4="executing: " set -x  # to inform what is going on
./autogen.sh --enable-werror
