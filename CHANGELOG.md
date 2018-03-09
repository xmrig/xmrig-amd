# v2.5.0
- [#434](https://github.com/xmrig/xmrig/issues/434) **Added support for Monero v7 PoW, scheduled on March 28.**
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
