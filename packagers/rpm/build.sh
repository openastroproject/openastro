#!/bin/bash

rel=`cut -d' ' -f3 < /etc/redhat-release`
fedpkg --release f$rel local
