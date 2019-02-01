## Windows

#### Recomendented drivers:
* [Adrenalin Edition 18.5.1](https://www.amd.com/en/support/kb/release-notes/rn-rad-win-18-5-1)
* [Adrenalin Edition 18.5.2](https://www.amd.com/en/support/kb/release-notes/rn-rad-win-18-5-2)
* [Adrenalin Edition 18.6.1](https://www.amd.com/en/support/kb/release-notes/rn-rad-win-18-6-1)

#### Known issues
* Blockchain drivers not recomendented to use for `cryptonight/2`.
* All drivers newer 18.6.1 **not work**, all shares will be rejected.

## Linux
#### Recomendented drivers:
* [ROCm 1.9.1+](https://github.com/RadeonOpenCompute/ROCm)

### AMD Vega
ROCm can't give same perfomance as Windows drivers, last working Linux driver [18.10](https://www.amd.com/en/support/kb/release-notes/rn-rad-pro-lin-18-10) even worse.
But possible get same or little better hashrate as Windows:

#### Steps:
* Download and [build](https://github.com/xmrig/xmrig-amd/wiki/Ubuntu-Build) xmrig-amd 2.8.4+ with cmake option `-DSTRICT_CACHE=OFF`.
* Install [18.10](https://www.amd.com/en/support/kb/release-notes/rn-rad-pro-lin-18-10) driver.
* Configure and run miner to create OpenCL cache in `.cache` directory near executable.
* Uninstall 18.10 driver and install [18.30](https://www.amd.com/en/support/kb/release-notes/rn-prorad-lin-18-30) driver, don't forget reboot.
* Run miner as usuall, if new versions of miner will contains changed OpenCL code, cache regeneration will be required again.
