# 1.9.5
- Rebase from XMRig-amd 2.15.4 (Fixed compatibility with recent AMD drivers (19.7.2), thanks @psychocrypt)
- Added possibility to delete templates #257
- Added embedded config parsing #256
- OSX Hugepages fix #250
- Fixed non-merged template assignment      
# 1.9.3
- Integrated UPX2 variant (algo: "cryptonight-extremelite", variant: "upx2" OR variant: "auto")
- Integrated merged templates and replace of @WORKER-ID@ in template assignment
# 1.9.1
- Rebase from XMRig-amd 2.14.0
- Integrated Monero CN-R variant so called CNv4, aka CN-R, aka CNv5, aka Cryptonight-R (algo: "cryptonight", variant: "r" / "auto")
- Integrated Wownero CN-R variant (algo: "cryptonight", variant: "wow")
- Integrated Graft variant (algo: "cryptonight", variant: "rwz" OR variant: "graft")
- Integrated X-Cash variant (algo: "cryptonight", variant: "double" OR variant: "heavyx" OR variant: "xcash")
- Integrated Zelerius variant (algo: "cryptonight", variant: "zls" OR variant: "zelerius")
- Add miner version column to the Dashboard (version turns red when its outdated)
- Fixed crash when remote logging is disabled
# 1.8.14
- Rebase from XMRig-amd 2.11.0
    - Integrated new RYO algo CN-GPU (algo: "cryptonight", variant: "gpu")
- Added alias CN-HOSP for RTO algo (algo: "cryptonight", variant: "hosp" or variant: "rto")    
# 1.8.12
- Fixed cn-ultralite no suitable algo error when using xmrig-proxy
- Added more names to the algo parse CN-Turtle/Ultralite
# 1.8.11
- Added fix for upcoming Masari fork (cn-half/fast2) (algo: "cryptonight", variant: "msr" (autodetect), "fast2" (force))
- Integrated cn-ultralite (turtleV2/DEGO) (algo: "cryptonight-ultralite", variant: "auto")
- Rebase from XMRig-amd 2.10.0
# 1.8.9
- Improved algo parsing for XTL v9 aka cn-half/fast2
# 1.8.8
- Added XLT v5/9 with autodetect(algo: "cryptonight", variant: "xtl" (autodetect), "fast2" (force v9))
- Added cn-lite variant UPX/uPlexa (algo: "cryptonight-lite", variant "upx")
# 1.8.7
- Implemented Template based mass config editor to simple swap configs on your rigs
# 1.8.6
- Integrated hashrate + cardcrash monitor to reboot/restart the miner
- Fixed Restart of miner via Dashboard when a card crashed
- Added start-cmd which is executed before the miner starts 
- Rebase from XMRig-amd 2.8.7-dev (THX xmrig!)
- Integrated Telegram push notifications
- Fixed multi miner editor
- Added miner offline/online status push notification
- Added 0/recovered hashrate push notification
# 1.8.5
- Add remote reboot (machine) feature to Dashboard, Server & Miner
- Integrated Pushover push notifications for Offline miners and periodical status notifications on iOS and Android
- Fixed XFH algo recognition send by Pool/Proxy
# 1.8.4
- Added XFH (Freehaven-project) support aka CN-Heavy-superfast (algo=cryptonight variant=xfh)
- Fix memory leak in cc client component
- Rebase from XMRig-amd 2.8.6 (Thanks xmrig)
    - Improved cn-heavy, cn-heavy/xhv perfomance up to 8% since v2.8.5 and up to 16% since v2.8.4, thanks @SChernykh
    - Fixed hashrate fluctuations. It's no longer necessary to use different intensities per thread.
    - Improved cn-heavy/tube perfomance up to 6% and cn/2 perfomance up to 1%.
    - Reduced power consumption with cn/2
    - Fixed possible invalid shares right after donation finish.
    - Improved AMD Vega64 auto configuration.
    - It's now recommended to revise your config.json and try:
    - Same intensities for both threads.
    - strided_index=2, mem_chunk=1 for cn/2.
    - strided_index=1 for other algorithms.   
# 1.8.0
- Rebase from XMRig-amd 2.8.4 (THX xmrig!)
    - Now we have full TLS support on Stratum+CC
    - Integration of CNV2 aka monero v8
- Fixed avg.time on Dashboard
- Fix supported-variants announcement in login 
# 1.7.0
- First official Release of XMRigCC-amd base on XMRig-amd 2.7.3-beta #33 #3
- Full integration of xmrigCC-amd into XMRigCCServer/Dashboard with GPUInfo / remote logging
- All features from XMRigCC 1.7.0 (except TLS on stratum)
    - Remote restart/start/pause/stop
    - Remote stats/info
    - Remote log
    - Remote configuration
    - ...  
