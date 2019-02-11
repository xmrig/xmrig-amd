# XCASH NVIDIA Miner

This program is based on XMRIG
* This is the **AMD-mining** version, there is also a [CPU version](https://github.com/X-CASH-official/XCASH_CPU_Miner) and [NVIDIA GPU version](https://github.com/X-CASH-official/XCASH_NVIDIA_Miner).

Note: There is 0% dev fee

#### Table of contents
* [Features](#features)
* [Download](#download)
* [Usage](#usage)
* [Example Usage](#example-usage)
* [Build](#build-instructions)

## Features
* High performance.
* Official Windows support.
* Support for backup (failover) mining server.
* Automatic GPU configuration.
* Nicehash support.
* It's open source software.

## Download
* Binary releases: https://github.com/X-CASH-official/XCASH_AMD_Miner/releases
* Git tree: https://github.com/X-CASH-official/XCASH_AMD_Miner.git
* Clone with `git clone hhttps://github.com/X-CASH-official/XCASH_AMDA_Miner.git`


## Usage

### Command line options
```
-a, --algo=ALGO              specify the algorithm to use
                                 cryptonight
                                 cryptonight-lite
                                 cryptonight-heavy
  -o, --url=URL                URL of mining server
  -O, --userpass=U:P           username:password pair for mining server
  -u, --user=USERNAME          username for mining server
  -p, --pass=PASSWORD          password for mining server
      --rig-id=ID              rig identifier for pool-side statistics (needs pool support)
  -k, --keepalive              send keepalived for prevent timeout (needs pool support)
      --nicehash               enable nicehash.com support
      --tls                    enable SSL/TLS support (needs pool support)
      --tls-fingerprint=F      pool TLS certificate fingerprint, if set enable strict certificate pinning
  -r, --retries=N              number of times to retry before switch to backup server (default: 5)
  -R, --retry-pause=N          time to pause between retries (default: 5)
      --opencl-devices=N       list of OpenCL devices to use.
      --opencl-launch=IxW      list of launch config, intensity and worksize
      --opencl-strided-index=N list of strided_index option values for each thread
      --opencl-mem-chunk=N     list of mem_chunk option values for each thread
      --opencl-comp-mode=N     list of comp_mode option values for each thread
      --opencl-affinity=N      list of affinity GPU threads to a CPU
      --opencl-platform=N      OpenCL platform index
      --opencl-loader=N        path to OpenCL-ICD-Loader (OpenCL.dll or libOpenCL.so)
      --print-platforms        print available OpenCL platforms and exit
      --no-cache               disable OpenCL cache
      --no-color               disable colored output
      --variant                algorithm PoW variant
      --donate-level=N         donate level, default 5% (5 minutes in 100 minutes)
      --user-agent             set custom user-agent string for pool
  -B, --background             run the miner in the background
  -c, --config=FILE            load a JSON-format configuration file
  -l, --log-file=FILE          log all output to a file
  -S, --syslog                 use system log for output messages
      --print-time=N           print hashrate report every N seconds
      --api-port=N             port for the miner API
      --api-access-token=T     access token for API
      --api-worker-id=ID       custom worker-id for API
      --api-id=ID              custom instance ID for API
      --api-ipv6               enable IPv6 support for API
      --api-no-restricted      enable full remote access (only if API token set)
      --dry-run                test configuration and exit
  -h, --help                   display this help and exit
  -V, --version                output version information and exit
```

## Example Usage
### Setup the enviroment variables

AMD usually works better when the following enviroment variables are set:  
```
GPU_FORCE_64BIT_PTR=1
GPU_MAX_HEAP_SIZE=100
GPU_MAX_ALLOC_PERCENT=100
GPU_SINGLE_ALLOC_PERCENT=100
```

You will need to set these enviroment varaible on each boot of the computer.

To set these on Windows type this in any command prompt:  
```
set GPU_FORCE_64BIT_PTR=1
set GPU_MAX_HEAP_SIZE=100
set GPU_MAX_ALLOC_PERCENT=100
set GPU_SINGLE_ALLOC_PERCENT=100
```

To set these on Linux type this in any terminal:  
```
export GPU_FORCE_64BIT_PTR=1
export GPU_MAX_HEAP_SIZE=100
export GPU_MAX_ALLOC_PERCENT=100
export GPU_SINGLE_ALLOC_PERCENT=100

### XCASH_AMD_Miner configuration
This is an example for a 3 VEGA mining computer
In this example, we are:
* Using the opencl platform 0 (try 1 and 2 if 0 gives an opencl error) `--opencl-platform=0`
* Setting our devices we want to use (we specifiy the device twice for VEGA to use two threads per card) `--opencl-devices=0,0,1,1,2,2`
* Setting the intensity to 1952 and the worksize to 16 per thread `--opencl-launch=1952x16,1952x16,1952x16,1952x16,1952x16,1952x16`
* Setting the strided_index to 2 per thread `--opencl-strided-index=2,2,2,2,2,2`
* Setting the mem_chunk to 2 per thread `--opencl-mem-chunk=2,2,2,2,2,2`
* Setting the comp_mode to true per thread `--opencl-comp-mode=true,true,true,true,true,true`
* Using keepalive for TCP packets to prevent a timeout `--keepalive`
* Connecting to the official X-CASH mining pool `--url minexcash.com:3333`
* Mining using "YOUR_XCASH_XCA_OR_XCB_ADDRESS" (replace YOUR_XCASH_XCA_OR_XCB_ADDRESS with your address) `--user YOUR_XCASH_XCA_OR_XCB_ADDRESS`
* Using a password of YOUR_MINING_COMPUTER_NAME (replace YOUR_MINING_COMPUTER_NAME with any name for your mining computer) `--pass YOUR_MINING_COMPUTER_NAME`  

`./XCASH_AMD_Miner --opencl-platform=0 --opencl-devices=0,0,1,1,2,2 --opencl-launch=1952x16,1952x16,1952x16,1952x16,1952x16,1952x16 --opencl-strided-index=2,2,2,2,2,2 --opencl-mem-chunk=2,2,2,2,2,2 --opencl-comp-mode=true,true,true,true,true,true --keepalive --url minexcash.com:3333 --user YOUR_XCASH_XCA_OR_XCB_ADDRESS --pass YOUR_MINING_COMPUTER_NAME`  

## Build Instructions

### Linux (Ubuntu)
* Install dependencies  
`sudo apt install git build-essential cmake libuv1-dev libmicrohttpd-dev libssl-dev`

* Clone the repository  
`git clone https://github.com/X-CASH-official/XCASH_AMD_Miner.git`

* Create a build folder and build the program  
`cd XCASH_AMD_Miner && mkdir build && cd build && cmake .. && make`

### Windows
* Install dependencies
Make sure you install Visual Studio 2017 Community Edition
https://visualstudio.microsoft.com/downloads/

Download prebuilt dependencies from XMRig and then unzip them anywhere
https://github.com/xmrig/xmrig-deps/releases
Note: If you just installed Visual Studio 2017 you can use 3.3, If you already had it installed try 3.1 instead

* Clone the repository   
`git clone https://github.com/X-CASH-official/XCASH_AMD_Miner.git`

* Create a build folder and build the program  
Replace PREBUILT_DEPENDENCIES_DIRECTORY with the directory location of the prebuilt dependencies  
`cd XCASH_AMD_Miner && mkdir build && cd build && cmake .. -G "Visual Studio 15 2017 Win64" -DXMRIG_DEPS=PREBUILT_DEPENDENCIES_DIRECTORY && cmake --build . --config Release`

If you get errors about redefintion of structs you might need to build without HTTPD and TLS (meaning the API and TSL will not work)  
`cd XCASH_AMD_Miner && mkdir build && cd build && cmake .. -G "Visual Studio 15 2017 Win64" -DXMRIG_DEPS=PREBUILT_DEPENDENCIES_DIRECTORY -DWITH_HTTPD=OFF -DWITH_TLS=OFF && cmake --build . --config Release`