## Sysrepo Helpers
### Show current Sysrepo configuration
- ```sysrepocfg -X -d [running|startup|operational] -m control-tsn-uni```

### Load initial data to Sysrepo
- ```sysrepocfg --import [file] -d [running|startup|operational] -m control-tsn-uni```
- e.g.: ```sysrepocfg --import=initial.xml -d running -m control-tsn-uni```

### Clean shared memory
1. cd to sysrepo repo 
2. cd to ```/sysrepo/build```
3. ```make shm_clean```

### Update YANG Model
1. Uninstall previous: ```sysrepoctl -u control-tsn-uni```
2. Install updated: ```sysrepoctl -i control-tsn-uni.yang```

### Update Sysrepo plugin
- run ```. plugin/update_plugin.sh```

### Start Sysrepo plugin manually
- ```sysrepo-plugind -v 4 -d```
    - v: Verbosity level
    - d: Debug mode (not going into daemon mode) and printing all messages to stderr