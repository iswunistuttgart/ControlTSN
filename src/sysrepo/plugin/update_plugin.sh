#!/bin/sh
# build
gcc -shared -o controltsn_plugin.so -fPIC controltsn_plugin.c

# copy to sysrepo plugin path
sudo cp controltsn_plugin.so /usr/local/lib/sysrepo/plugins