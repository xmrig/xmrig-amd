# XMRigCC-amd (OpenCL)

:warning: **Confused by all the forks? Check the [Coin Configuration](https://github.com/Bendr0id/xmrigCC/wiki/Coin-configurations) guide.**

:bulb: **This is the AMD GPU (OpenCL) variant of XMRigCC, if you're looking for the CPU variant [click here](https://github.com/Bendr0id/xmrigCC/).**


[![GitHub release](https://img.shields.io/github/release/bendr0id/xmrigCC-amd/all.svg)](https://github.com/bendr0id/xmrigCC-amd/releases)
[![Github downloads latest](https://img.shields.io/github/downloads/bendr0id/xmrigCC-amd/latest/total.svg)](https://github.com/bendr0id/xmrigCC-amd/releases)
[![Github downloads total](https://img.shields.io/github/downloads/bendr0id/xmrigCC-amd/total.svg)](https://github.com/bendr0id/xmrigCC-amd/releases)
[![GitHub stars](https://img.shields.io/github/stars/bendr0id/xmrigCC-amd.svg)](https://github.com/bendr0id/xmrigCC-amd/stargazers)

![XMRigCC Logo](https://i.imgur.com/7mi0WCe.png)


### About XMRigCC-amd (OpenCL)

XMRigCC-amd is a fork of [XMRig-amd](https://github.com/xmrig/xmrig-amd) which adds the ability to remote control your XMRig-amd instances via a Webfrontend and REST api.
This fork is based on XMRig-amd and adds a "Command and Control" (C&amp;C) server, a daemon to reload the miner on config changes and modifications in XMRig-amd to send the current status to the C&amp;C Server.
The modified version can also handle commands like "update config", "start/stop mining" or "restart/shutdown" which can be send from the C&amp;C-Server.

GPU mining part based on [Wolf9466](https://github.com/OhGodAPet) and [psychocrypt](https://github.com/psychocrypt) code.


:warning: Suggested values for GPU auto configuration can be not optimal or not working, you may need tweak your threads options. Please fell free open an [issue](https://github.com/bendr0id/xmrigCC-amd/issues) if auto configuration suggest wrong values.

**[Find Help/Howto](https://github.com/Bendr0id/xmrigCC/wiki/)**


**XMRigCC Daemon(miner)**

![Screenshot of XMRig Daemon (miner)](https://i.imgur.com/48uGuDI.jpg)

**XMRigCC Server**

![Screenshot of XMRigCC Server](https://i.imgur.com/iS1RzgO.png)

**XMRigCC Dashboard**

![Screenshot of XMRigCC Dashboard](https://imgur.com/UrdTHpM.png)


## Download
* Binary releases: https://github.com/Bendr0id/xmrigCC-amd/releases
* Git tree: https://github.com/Bendr0id/xmrigCC-amd.git
  * Clone with `git clone https://github.com/Bendr0id/xmrigCC.git` :hammer: [Build instructions](https://github.com/Bendr0id/xmrigCC/wiki/Build-Debian%5CUbuntu).

## Usage
### Basic example xmrigCCServer
```
xmrigCCServer --cc-port=3344 --cc-user=admin --cc-pass=pass --cc-access-token=SECRET_TOKEN_TO_ACCESS_CC_SERVER
```

### Options xmrigCCServer
```
        --cc-user=USERNAME                CC Server admin user
        --cc-pass=PASSWORD                CC Server admin pass
        --cc-access-token=T               CC Server access token for CC Client
        --cc-port=N                       CC Server
        --cc-use-tls                      enable tls encryption for CC communication
        --cc-cert-file=FILE               when tls is turned on, use this to point to the right cert file (default: server.pem) 
        --cc-key-file=FILE                when tls is turned on, use this to point to the right key file (default: server.key) 
        --cc-client-config-folder=FOLDER  Folder contains the client config files
        --cc-custom-dashboard=FILE        loads a custom dashboard and serve it to '/'
        --no-color                        disable colored output
        --cc-client-log-lines-history=N   maximum lines of log history kept per miner (default: 100)
    -S, --syslog                          use system log for output messages
    -B, --background                      run the miner in the background
    -c, --config=FILE                     load a JSON-format configuration file
    -l, --log-file=FILE                   log all output to a file
    -h, --help                            display this help and exit
    -V, --version                         output version information and exit
```

Also you can use configuration via config file, default **[config_cc.json](https://github.com/Bendr0id/xmrigCC/wiki/Config-XMRigCCServer)**. You can load multiple config files and combine it with command line options.

### Basic example xmrigDaemon
```
xmrigDaemon -o pool.minemonero.pro:5555 -u YOUR_WALLET -p x -k --cc-url=IP_OF_CC_SERVER:PORT --cc-access-token=SECRET_TOKEN_TO_ACCESS_CC_SERVER --cc-worker-id=OPTIONAL_WORKER_NAME
```

### Options xmrigDaemon
```
  -a, --algo=ALGO               cryptonight (default) or cryptonight-lite
  -o, --url=URL                 URL of mining server
  -O, --userpass=U:P            username:password pair for mining server
  -u, --user=USERNAME           username for mining server
  -p, --pass=PASSWORD           password for mining server
  -k, --keepalive               send keepalived for prevent timeout (need pool support)
  -r, --retries=N               number of times to retry before switch to backup server (default: 5)
  -R, --retry-pause=N           time to pause between retries (default: 5)
      --opencl-devices=N        list of OpenCL devices to use.
      --opencl-launch=IxW       list of launch config, intensity and worksize
      --opencl-affinity=N       affine GPU threads to a CPU
      --opencl-platform=N       OpenCL platform index
      --no-color                disable colored output
      --donate-level=N          donate level, default 5% (5 minutes in 100 minutes)
      --user-agent              set custom user-agent string for pool
  -B, --background              run the miner in the background
  -c, --config=FILE             load a JSON-format configuration file
  -l, --log-file=FILE           log all output to a file
      --nicehash                enable nicehash support
      --print-time=N            print hashrate report every N seconds
      --api-port=N              port for the miner API
      --api-access-token=T      access token for API
      --api-worker-id=ID        custom worker-id for API
      --cc-url=URL              url of the CC Server
      --cc-use-tls              enable tls encryption for CC communication
      --cc-access-token=T       access token for CC Server
      --cc-worker-id=ID         custom worker-id for CC Server
      --cc-update-interval-s=N  status update interval in seconds (default: 10 min: 1)
  -h, --help                    display this help and exit
  -V, --version                 output version information and exit
```

Also you can use configuration via config file, default **[config.json](https://github.com/Bendr0id/xmrigCC/wiki/Config-XMRigDaemon)**. You can load multiple config files and combine it with command line options.

## Common Issues
### XMRigMiner
* XMRigMiner is just the worker, it is not designed to work standalone. Please start **XMRigDaemon** instead.

### Windows only: DLL error on starting
* Make sure that you installed latest Visual C++ Redistributable for Visual Studio 2015. Can be downloaded here: [microsoft.com](https://www.microsoft.com/de-de/download/details.aspx?id=48145)

## Donations
* Default donation 5% (5 minutes in 100 minutes) can be reduced to 1% via command line option `--donate-level`. 

##### BenDroid (xmrigCC):
XMR:  `4BEn3sSa2SsHBcwa9dNdKnGvvbyHPABr2JzoY7omn7DA2hPv84pVFvwDrcwMCWgz3dQVcrkw3gE9aTC9Mi5HxzkfF9ev1eH`

AEON: `Wmtm4S2cQ8uEBBAVjvbiaVAPv2d6gA1mAUmBmjna4VF7VixLxLRUYag5cvsym3WnuzdJ9zvhQ3Xwa8gWxPDPRfcQ3AUkYra3W`

BTC:  `3Gwq9tveCZtLAiXX7mxhjbrh38GPx1iXdB`

##### xmrig:
XMR:  `48edfHu7V9Z84YzzMa6fUueoELZ9ZRXq9VetWzYGzKt52XU5xvqgzYnDK9URnRoJMk1j8nLwEVsaSWJ4fhdUyZijBGUicoD`

BTC:  `1P7ujsXeX7GxQwHNnJsRMgAdNkFZmNVqJT`

## Contact
* ben [at] graef.in
* Telegram: @BenDr0id
* [discord](https://discord.gg/r3rCKTB)
* [reddit](https://www.reddit.com/user/BenDr0id/)
