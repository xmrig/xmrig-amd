# v2.14.0
- **[#227](https://github.com/xmrig/xmrig-amd/pull/227) Added new algorithm `cryptonight/rwz`, short alias `cn/rwz` (also known as CryptoNight ReverseWaltz), for upcoming [Graft](https://www.graft.network/) fork.**
- **[#931](https://github.com/xmrig/xmrig/issues/931) Added new algorithm `cryptonight/zls`, short alias `cn/zls` for [Zelerius Network](https://zelerius.org) fork.**
- **[#940](https://github.com/xmrig/xmrig/issues/940) Added new algorithm `cryptonight/double`, short alias `cn/double` (also known as CryptoNight HeavyX), for [X-CASH](https://x-cash.org/).**
- [#951](https://github.com/xmrig/xmrig/issues/951#issuecomment-469581529) Fixed crash if AVX was disabled on OS level.
- [#952](https://github.com/xmrig/xmrig/issues/952) Fixed compile error on some Linux.
- [#957](https://github.com/xmrig/xmrig/issues/957#issuecomment-468890667) Added support for embedded config.

# v2.13.0
- **[#938](https://github.com/xmrig/xmrig/issues/938) Added support for new algorithm `cryptonight/r`, short alias `cn/r` (also known as CryptoNightR or CryptoNight variant 4), for upcoming [Monero](https://www.getmonero.org/) fork on March 9, thanks [@SChernykh](https://github.com/SChernykh).**
- [#939](https://github.com/xmrig/xmrig/issues/939) Added support for dynamic (runtime) pools reload.
- Invalid threads (eg with wrong device index) now ignored and not stop the miner.

# v2.12.0
- [#218](https://github.com/xmrig/xmrig-amd/pull/218) Added support for new algorithm `cryptonight/wow`, short alias `cn/wow` (also known as CryptonightR), for upcoming [Wownero](http://wownero.org) fork on February 14.
- Improved `cryptonight/gpu` performance.

# v2.11.1
* Fixed regression, algorithm `cn-pico/trtl` was broken in v2.11.0.

# v2.11.0
- [#928](https://github.com/xmrig/xmrig/issues/928) Added support for new algorithm `cryptonight/gpu`, short alias `cn/gpu` (original name `cryptonight-gpu`), for upcoming [Ryo currency](https://ryo-currency.com) fork on February 14.
- Fixed compatibility with AMD drivers, latest Windows/Linux drivers now supported.

# v2.10.0
- [#904](https://github.com/xmrig/xmrig/issues/904) Added new algorithm `cn-pico/trtl` (aliases `cryptonight-turtle`, `cn-trtl`) for upcoming TurtleCoin (TRTL) fork.

# v2.9.4
- [#913](https://github.com/xmrig/xmrig/issues/913) Fixed Masari (MSR) support (this update required for upcoming fork).

# v2.9.3
- [#211](https://github.com/xmrig/xmrig-amd/pull/211) Fixed `cn/half` compute errors.
- Removed verbose messages about threads interleave.

# v2.9.1
- [#899](https://github.com/xmrig/xmrig/issues/899) Added support for new algorithm `cn/half` for Masari and Stellite forks.
- [#203](https://github.com/xmrig/xmrig-amd/pull/203) Fixed GPU errors with worksize != 8 on cn-heavy.

# v2.8.6
- **Improved `cn-heavy`, `cn-heavy/xhv` perfomance up to 8% since v2.8.5 and up to 16% since v2.8.4, thanks [@SChernykh](https://github.com/SChernykh)**, pull requests [#187](https://github.com/xmrig/xmrig-amd/pull/187), [#189](https://github.com/xmrig/xmrig-amd/pull/189), [#190](https://github.com/xmrig/xmrig-amd/pull/190), [#191](https://github.com/xmrig/xmrig-amd/pull/191), [#192](https://github.com/xmrig/xmrig-amd/pull/192) and [#193](https://github.com/xmrig/xmrig-amd/pull/193).
- **[#195](https://github.com/xmrig/xmrig-amd/pull/195) Fixed hashrate fluctuations. It's no longer necessary to use different intensities per thread.**
- Improved `cn-heavy/tube` perfomance up to 6% and `cn/2` perfomance up to 1%.
- Reduced power consumption with `cn/2`
- Fixed possible invalid shares right after donation finish.
- Improved AMD Vega64 auto configuration.
- It's now recommended to revise your `config.json` and try:
  - Same intensities for both threads.
  - `strided_index=2, mem_chunk=1` for `cn/2`.
  - `strided_index=1` for other algorithms.

# v2.8.5
- [#185](https://github.com/xmrig/xmrig-amd/pull/185) **Improved `cn-heavy`, `cn-heavy/xhv` and `cn-heavy/tube` perfomance up to 8%, thanks [@SChernykh](https://github.com/SChernykh).**
- [#271](https://github.com/xmrig/xmrig-proxy/issues/271) Fixed pool options cascading when use mixed configuration: config file and command line.
- Improved AMD Vega 56 auto configuration for `cn/2`.

# v2.8.4
- **Improved AMD Vega autoconfig (double threads & higher intensity).**
- Fixed broken OpenCL code for `cn-lite` and `cn-heavy` (regression since v2.8.2).
- [#166](https://github.com/xmrig/xmrig-amd/pull/166) Fixed graceful OpenCL shutdown.
- Fixed OpenCL compile warnig with ROCm 1.9.1.
- OpenCL cache file name now displayed in when strict cache disabled (`-DSTRICT_CACHE=OFF`).
- Fixed wrong displayed GPU name in autoconfig phase.

# v2.8.3
- [#813](https://github.com/xmrig/xmrig/issues/813) Fixed critical bug with Minergate pool and variant 2.

# v2.8.2
- [#167](https://github.com/xmrig/xmrig-amd/issues/167) Fixed wrong hashrate in `GET /1/threads` API endpoint.
- [#168](https://github.com/xmrig/xmrig-amd/issues/168) Fixed broken AMD OpenCL compile (old driver bug).

# v2.8.1
- [#156](https://github.com/xmrig/xmrig-amd/issues/156) Added CMake option to disable strict OpenCL cache.
- [#769](https://github.com/xmrig/xmrig/issues/769) Fixed regression, some ANSI escape sequences was in log with disabled colors.
- [#777](https://github.com/xmrig/xmrig/issues/777) Better report about pool connection issues. 
- Added missing options to `--help` output.

# v2.8.0
- **[#753](https://github.com/xmrig/xmrig/issues/753) Added new algorithm [CryptoNight variant 2](https://github.com/xmrig/xmrig/issues/753) for Monero fork, thanks [@SChernykh](https://github.com/SChernykh).**
- [#758](https://github.com/xmrig/xmrig/issues/758) **Added SSL/TLS support for secure connections to pools.**
  - Added per pool options `"tls"` and `"tls-fingerprint"` and command line equivalents.
- [#162](https://github.com/xmrig/xmrig-amd/issues/162) Extended `opencl-platform` option.
- [#767](https://github.com/xmrig/xmrig/issues/767) Added `autosave` config option.
- [#245](https://github.com/xmrig/xmrig-proxy/issues/245) Fixed API ID collision when run multiple miners on same machine.
- [#757](https://github.com/xmrig/xmrig/issues/757) Fixed send buffer overflow.

# v2.7.3-beta
- [#145](https://github.com/xmrig/xmrig-amd/issues/145) Added runtime linking with OpenCL ICD, **AMD APP SDK not required anymore**.
- [#140](https://github.com/xmrig/xmrig-amd/issues/140) `cryptonight-lite/ipbc` replaced to `cryptonight-heavy/tube` for **Bittube (TUBE)**.
- [#128](https://github.com/xmrig/xmrig-amd/issues/128) Improved `cryptonight/msr` support, removed usage restrictions.
- Added `cryptonight/rto` (cryptonight variant 1 with IPBC/TUBE mod) variant for **Arto (RTO)** coin.
- Added `cryptonight/xao` (original cryptonight with bigger iteration count) variant for **Alloy (XAO)** coin.
- Added option `opencl-loader` for custom path to OpenCL ICD.
- Vega APU (AMD Ryzen with embedded GPU) now excluded from autoconfig, reason: slow and cause BSOD.

# v2.7.2-beta
- [#132](https://github.com/xmrig/xmrig-amd/issues/132) Fixed regression, command line option `--opencl-platform` was broken.

# v2.7.1-beta
- [#130](https://github.com/xmrig/xmrig-amd/issues/130) **Added OpenCL cache support**.
  - Added config option `cache` and command line option `--no-cache` to allow disable cache.
- **Added support for new cryptonight-heavy variant xhv** (`cn-heavy/xhv`) for Haven Protocol fork.
- [#128](https://github.com/xmrig/xmrig-amd/issues/128) **Added support for new cryptonight variant msr** (`cn/msr`) also known as `cryptonight-fast` for Masari fork.
- [#126](https://github.com/xmrig/xmrig-amd/issues/126) Fixed regression, command line option `--print-platforms` was broken.
- [#127](https://github.com/xmrig/xmrig-amd/issues/127) Fixed regression, miner was not exit if OpenCL errors happen.

# v2.7.0-beta
- **Added support for cryptonight-lite variant ipbc** (`cn-lite/ipbc`) for BitTube also was known as IPBC.
- **Added support for cryptonight variant xtl** (`cn/xtl`) for Stellite.
- Added [config options](https://github.com/xmrig/xmrig-amd/blob/dev/doc/THREADS.md) `strided_index`, `mem_chunk` and `comp_mode`.
- Added new detailed hashrate report.
- Added command line option `--dry-run`.

# v2.6.1
- Fixed critical bug, in some cases miner was can't recovery connection and switch to failover pool, version 2.5.2 and v2.6.0-beta1 affected.
- [#499](https://github.com/xmrig/xmrig/issues/499) IPv6 support disabled for internal HTTP API.
- Added workaround for nicehash.com if you use `cryptonightv7.<region>.nicehash.com` option `variant=1` will be set automatically.

# v2.6.0-beta1
 - [#476](https://github.com/xmrig/xmrig/issues/476) **Added Cryptonight-Heavy support for Sumokoin ASIC resistance fork.**
 
# v2.5.2
- [#448](https://github.com/xmrig/xmrig/issues/478) Fixed broken reconnect.

# v2.5.1
- [#454](https://github.com/xmrig/xmrig/issues/454) Fixed build with libmicrohttpd version below v0.9.35.
- [#456](https://github.com/xmrig/xmrig/issues/459) Verbose errors related to donation pool was not fully silenced.
- [#459](https://github.com/xmrig/xmrig/issues/459) Fixed regression (version 2.5.0 affected) with connection to **xmr.f2pool.com**.

# v2.5.0
- [#434](https://github.com/xmrig/xmrig/issues/434) **Added support for Monero v7 PoW, scheduled on April 6.**
- Added full IPv6 support.
- Added protocol extension, when use the miner with xmrig-proxy 2.5+ no more need manually specify `nicehash` option.
- [#51](https://github.com/xmrig/xmrig-amd/issues/51) Fixed multiple pools in initial config was saved incorrectly.
- [#123](https://github.com/xmrig/xmrig-proxy/issues/123) Fixed regression (all versions since 2.4 affected) fragmented responses from pool/proxy was parsed incorrectly.

# v2.4.5
 - [#49](https://github.com/xmrig/xmrig-amd/issues/49) Fixed, in some cases, pause was cause an infinite loop.
 - [#200](https://github.com/xmrig/xmrig/issues/200) In some cases miner was doesn't write log to stdout.
 - Added libmicrohttpd version to --version output.
 - Fixed bug in singal handler, in some cases miner wasn't shutdown properly.
 - Fixed recent MSVC 2017 version detection.

# v2.4.3-beta2
 - Fixed, auto config wasn't write opencl-platform to config.json.
 - Added command line option `--print-platforms`.
 - Fixed 32 bit build.
 - [#2](https://github.com/xmrig/xmrig-amd/issues/2) Fixed Linux build.
 - [#3](https://github.com/xmrig/xmrig-amd/issues/3) Fixed macOS build.

# v2.4.3-beta1
 - First public release.
