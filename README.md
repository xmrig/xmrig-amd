# XMRig
XMRig is high performance Monero (XMR) OpenCL miner, with the official full Windows support.

GPU mining part based on [Wolf9466](https://github.com/OhGodAPet) and [psychocrypt](https://github.com/psychocrypt) code.

* This is the AMD (OpenCL) GPU mining version, there is also a [CPU version](https://github.com/xmrig/xmrig) and [NVIDIA GPU version](https://github.com/xmrig/xmrig-nvidia).
* [Roadmap](https://github.com/xmrig/xmrig/issues/106) for next releases.

#### Table of contents
* [Features](#features)
* [Download](#download)
* [Usage](#usage)
* [Build](https://github.com/xmrig/xmrig-amd/wiki/Build)
* [Donations](#donations)
* [Contacts](#contacts)

## Features
* High performance.
* Official Windows support.
* Support for backup (failover) mining server.
* CryptoNight-Lite support for AEON.
* Automatic GPU configuration.
* Nicehash support.
* It's open source software.

## Download
* Binary releases: https://github.com/xmrig/xmrig-amd/releases
* Git tree: https://github.com/xmrig/xmrig-amd.git
  * Clone with `git clone https://github.com/xmrig/xmrig-amd.git`  :hammer: [Build instructions](https://github.com/xmrig/xmrig-amd/wiki/Build).

## Usage

### Command line options
```
  -a, --algo=ALGO           cryptonight (default) or cryptonight-lite
  -o, --url=URL             URL of mining server
  -O, --userpass=U:P        username:password pair for mining server
  -u, --user=USERNAME       username for mining server
  -p, --pass=PASSWORD       password for mining server
  -k, --keepalive           send keepalived for prevent timeout (need pool support)
  -r, --retries=N           number of times to retry before switch to backup server (default: 5)
  -R, --retry-pause=N       time to pause between retries (default: 5)
      --opencl-devices=N    list of OpenCL devices to use.
      --opencl-launch=IxW   list of launch config, intensity and worksize
      --opencl-affinity=N   affine GPU threads to a CPU
      --opencl-platform=N   OpenCL platform index
      --no-color            disable colored output
      --donate-level=N      donate level, default 5% (5 minutes in 100 minutes)
      --user-agent          set custom user-agent string for pool
  -B, --background          run the miner in the background
  -c, --config=FILE         load a JSON-format configuration file
  -l, --log-file=FILE       log all output to a file
      --nicehash            enable nicehash support
      --print-time=N        print hashrate report every N seconds
      --api-port=N          port for the miner API
      --api-access-token=T  access token for API
      --api-worker-id=ID    custom worker-id for API
  -h, --help                display this help and exit
  -V, --version             output version information and exit
```

### Config file.
GPU configuration now possible only via config file. Sample config:
```json
{
    "algo": "cryptonight",
    "background": false,
    "colors": true,
    "donate-level": 5,
    "log-file": null,
    "print-time": 60,
    "retries": 5,
    "retry-pause": 5,
    "syslog": false,
    "opencl-platform": 0,
    "threads": [
        {
            "index": 0,
            "intensity": 896,
            "worksize": 8,
            "affine_to_cpu": false
        }
    ],
    "pools": [
        {
            "url": "pool.monero.hashvault.pro:5555",
            "user": "",
            "pass": "x",
            "keepalive": true,
            "nicehash": false
        }
    ],
    "api": {
        "port": 0,
        "access-token": null,
        "worker-id": null
    }
}
```
If `threads` option not specified the miner will try automatically create optimal configuration for your AMD GPUs.

## Donations
Default donation 5% (5 minutes in 100 minutes) can be reduced to 1% via command line option `--donate-level`.

* XMR: `48edfHu7V9Z84YzzMa6fUueoELZ9ZRXq9VetWzYGzKt52XU5xvqgzYnDK9URnRoJMk1j8nLwEVsaSWJ4fhdUyZijBGUicoD`
* BTC: `1P7ujsXeX7GxQwHNnJsRMgAdNkFZmNVqJT`

## Contacts
* support@xmrig.com
* [reddit](https://www.reddit.com/user/XMRig/)
