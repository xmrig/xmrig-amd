cmake_minimum_required(VERSION 2.8)
project(xmrig)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)

option(WITH_AEON     "CryptoNight-Lite support" ON)
option(WITH_SUMO     "CryptoNight-Heavy support" ON)
option(WITH_HTTPD    "HTTP REST API" ON)
option(WITH_CC_CLIENT "CC Client" ON)
option(WITH_TLS       "TLS support" ON)
option(BUILD_STATIC  "Build static binary" OFF)
option(WITH_CN_PICO   "CryptoNight-Pico support" ON)
option(WITH_CN_GPU    "CryptoNight-GPU support" ON)
option(WITH_CN_EXTREMELITE "CryptoNight-Extremelite support" ON)
option(WITH_ASM       "Enable ASM PoW implementations" ON)
option(STRICT_CACHE   "Enable strict checks for OpenCL cache" ON)
option(ARM_TARGET     "Force use specific ARM target 8 or 7" 0)

option(WITH_DEBUG_LOG            "Enable debug log output, network, etc" OFF)
option(WITH_EMBEDDED_CONFIG      "Enable internal embedded JSON config" OFF)
option(WITH_INTERLEAVE_DEBUG_LOG "Enable debug log for threads interleave" ON)
if(NOT MINER_EXECUTABLE_NAME)
    set(MINER_EXECUTABLE_NAME "xmrigMiner-amd" CACHE STRING "Miner executable file name")
endif(NOT MINER_EXECUTABLE_NAME)

if(NOT DAEMON_EXECUTABLE_NAME)
    set(DAEMON_EXECUTABLE_NAME "xmrigDaemon-amd" CACHE STRING "Daemon executable file name")
endif(NOT DAEMON_EXECUTABLE_NAME)

include (CheckIncludeFile)
include (cmake/cpu.cmake)


set(HEADERS
    src/amd/cryptonight.h
    src/amd/GpuContext.h
    src/amd/OclCache.h
    src/amd/OclCLI.h
    src/amd/OclCryptonightR_gen.h
    src/amd/OclError.h
    src/amd/OclGPU.h
    src/amd/OclLib.h
    src/api/NetworkState.h
    src/App.h
    src/base/io/Json.h
    src/base/io/Watcher.h
    src/base/kernel/Entry.h
    src/base/kernel/interfaces/IConfigListener.h
    src/base/kernel/interfaces/ISignalListener.h
    src/base/kernel/interfaces/IWatcherListener.h
    src/base/kernel/Process.h
    src/base/kernel/Signals.h
    src/base/net/Pool.h
    src/base/net/Pools.h
    src/base/tools/Arguments.h
    src/base/tools/Handle.h
    src/base/tools/String.h
    src/common/config/CommonConfig.h
    src/common/config/ConfigLoader.h
    src/common/config/ConfigWatcher.h
    src/common/Console.h
    src/common/cpu/BasicCpuInfo.h
    src/common/cpu/Cpu.h
    src/common/crypto/Algorithm.h
    src/common/crypto/keccak.h
    src/common/interfaces/IClientListener.h
    src/common/interfaces/IConfig.h
    src/common/interfaces/IConfigCreator.h
    src/common/interfaces/IConsoleListener.h
    src/common/interfaces/IControllerListener.h
    src/common/interfaces/ICpuInfo.h
    src/common/interfaces/ILogBackend.h
    src/common/interfaces/IStrategy.h
    src/common/interfaces/IStrategyListener.h
    src/common/log/BasicLog.h
    src/common/log/RemoteLog.h
    src/common/log/ConsoleLog.h
    src/common/log/FileLog.h
    src/common/log/Log.h
    src/common/net/Client.h
    src/common/net/Id.h
    src/common/net/Job.h
    src/common/net/Storage.h
    src/common/net/strategies/FailoverStrategy.h
    src/common/net/strategies/SinglePoolStrategy.h
    src/common/net/SubmitResult.h
    src/common/Platform.h
    src/common/utils/c_str.h
    src/common/utils/mm_malloc.h
    src/common/utils/timestamp.h
    src/common/xmrig.h
    src/core/ConfigLoader_default.h
    src/core/ConfigLoader_platform.h
    src/core/Controller.h
    src/core/usage.h
    src/interfaces/IJobResultListener.h
    src/interfaces/IThread.h
    src/interfaces/IWorker.h
    src/Mem.h
    src/net/JobResult.h
    src/net/Network.h
    src/net/strategies/DonateStrategy.h
    src/Summary.h
    src/version.h
    src/workers/Handle.h
    src/workers/Hashrate.h
    src/workers/OclThread.h
    src/workers/OclWorker.h
    src/workers/Workers.h
   )

set(HEADERS_CRYPTO
    src/crypto/c_blake256.h
    src/crypto/c_groestl.h
    src/crypto/c_jh.h
    src/crypto/c_skein.h
    src/crypto/CryptoNight.h
    src/crypto/CryptoNight_constants.h
    src/crypto/CryptoNight_monero.h
    src/crypto/CryptoNight_test.h
    src/crypto/groestl_tables.h
    src/crypto/hash.h
    src/crypto/skein_port.h
    src/crypto/soft_aes.h
   )
   
if (XMRIG_ARM)
    set(HEADERS_CRYPTO "${HEADERS_CRYPTO}" src/crypto/CryptoNight_arm.h)
else()
    set(HEADERS_CRYPTO "${HEADERS_CRYPTO}" src/crypto/CryptoNight_x86.h)
endif()

set(SOURCES
    src/amd/OclCache.cpp
    src/amd/OclCLI.cpp
    src/amd/OclCryptonightR_gen.cpp
    src/amd/OclGPU.cpp
    src/amd/OclLib.cpp
    src/api/NetworkState.cpp
    src/App.cpp
    src/base/io/Json.cpp
    src/base/io/Watcher.cpp
    src/base/kernel/Entry.cpp
    src/base/kernel/Process.cpp
    src/base/kernel/Signals.cpp
    src/base/net/Pool.cpp
    src/base/net/Pools.cpp
    src/base/tools/Arguments.cpp
    src/base/tools/Handle.cpp
    src/base/tools/String.cpp
    src/common/config/CommonConfig.cpp
    src/common/config/ConfigLoader.cpp
    src/common/config/ConfigWatcher.cpp
    src/common/Console.cpp
    src/common/cpu/BasicCpuInfo.cpp
    src/common/cpu/Cpu.cpp
    src/common/crypto/Algorithm.cpp
    src/common/crypto/keccak.cpp
    src/common/net/Client.cpp
    src/common/net/Job.cpp
    src/common/net/strategies/FailoverStrategy.cpp
    src/common/net/strategies/SinglePoolStrategy.cpp
    src/common/net/SubmitResult.cpp
    src/common/Platform.cpp
    src/core/Config.cpp
    src/core/Controller.cpp
    src/Mem.cpp
    src/net/Network.cpp
    src/net/strategies/DonateStrategy.cpp
    src/Summary.cpp
    src/workers/Handle.cpp
    src/workers/Hashrate.cpp
    src/workers/OclThread.cpp
    src/workers/OclWorker.cpp
    src/workers/Workers.cpp
    src/workers/HashrateMonitor.cpp
    src/xmrig.cpp
   )

set(SOURCES_COMMON
        src/common/Console.cpp
        src/common/config/CommonConfig.cpp
        src/common/config/ConfigLoader.cpp
        src/common/config/ConfigWatcher.cpp
        src/common/log/ConsoleLog.cpp
        src/common/log/RemoteLog.cpp
        src/common/log/FileLog.cpp
        src/common/log/Log.cpp
        src/common/log/BasicLog.cpp
        src/common/Platform.cpp
        src/core/Config.cpp
        src/core/Controller.cpp
    )


set(SOURCES_CRYPTO
    src/crypto/c_groestl.c
    src/crypto/c_blake256.c
    src/crypto/c_jh.c
    src/crypto/c_skein.c
    src/crypto/CryptoNight.cpp
   )

if (WITH_ASM)
    set(HEADERS_CRYPTO "${HEADERS_CRYPTO}" src/crypto/asm/CryptonightR_template.h)
    set(SOURCES_CRYPTO "${SOURCES_CRYPTO}" src/crypto/CryptonightR_gen.cpp)
endif()

if (WIN32)
    set(SOURCES_OS
        res/app.rc
        src/amd/OclCache_win.cpp
        src/App_win.cpp
        src/base/io/Json_win.cpp
        src/common/Platform_win.cpp
        src/Mem_win.cpp
        )

    add_definitions(/DWIN32)
    set(EXTRA_LIBS ws2_32 psapi iphlpapi userenv crypt32 winmm)
elseif (APPLE)
    set(SOURCES_OS
        src/amd/OclCache_unix.cpp
        src/App_unix.cpp
        src/base/io/Json_unix.cpp
        src/common/Platform_mac.cpp
        src/Mem_unix.cpp
        )
else()
    set(SOURCES_OS
        src/amd/OclCache_unix.cpp
        src/App_unix.cpp
        src/base/io/Json_unix.cpp
        src/common/Platform_unix.cpp
        src/Mem_unix.cpp
        )

    set(EXTRA_LIBS pthread rt dl)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL FreeBSD)
    set(EXTRA_LIBS ${EXTRA_LIBS} kvm)
endif()

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    EXECUTE_PROCESS(COMMAND uname -o COMMAND tr -d '\n' OUTPUT_VARIABLE OPERATING_SYSTEM)
    if (OPERATING_SYSTEM MATCHES "Android")
        set(EXTRA_LIBS ${EXTRA_LIBS} log)
    endif()
endif()

add_definitions(/D__STDC_FORMAT_MACROS)
add_definitions(/DUNICODE)
add_definitions(/DXMRIG_AMD_PROJECT)
add_definitions(/DXMRIG_NO_LIBCPUID)
add_definitions(/DMINER_EXECUTABLE_NAME=${MINER_EXECUTABLE_NAME})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

find_package(UV REQUIRED)

include(cmake/flags.cmake)

add_definitions(/DCL_TARGET_OPENCL_VERSION=200)
add_definitions(/DCL_USE_DEPRECATED_OPENCL_1_2_APIS)


include(cmake/cn-gpu.cmake)
include(cmake/asm.cmake)

CHECK_INCLUDE_FILE (syslog.h HAVE_SYSLOG_H)
if (HAVE_SYSLOG_H)
    add_definitions(/DHAVE_SYSLOG_H)
    set(SOURCES_SYSLOG src/common/log/SysLog.h src/common/log/SysLog.cpp)
endif()

if (STRICT_CACHE)
    add_definitions(/DXMRIG_STRICT_OPENCL_CACHE)
else()
    remove_definitions(/DXMRIG_STRICT_OPENCL_CACHE)
endif()

if (NOT WITH_AEON)
    add_definitions(/DXMRIG_NO_AEON)
endif()

if (NOT WITH_SUMO)
    add_definitions(/DXMRIG_NO_SUMO)
endif()

if (NOT WITH_CN_PICO)
    add_definitions(/DXMRIG_NO_CN_ULTRALITE)
endif()

if (NOT WITH_CN_EXTREMELITE)
    add_definitions(/DXMRIG_NO_CN_EXTREMELITE)
endif()

if (WITH_EMBEDDED_CONFIG)
    add_definitions(/DXMRIG_FEATURE_EMBEDDED_CONFIG)
endif()

if (WITH_HTTPD)
    find_package(MHD)

    if (MHD_FOUND)
        include_directories(${MHD_INCLUDE_DIRS})
        set(HTTPD_SOURCES
            src/api/Api.h
            src/api/ApiRouter.h
            src/common/api/HttpBody.h
            src/common/api/Httpd.h
            src/common/api/HttpReply.h
            src/common/api/HttpRequest.h
            src/api/Api.cpp
            src/api/ApiRouter.cpp
            src/common/api/Httpd.cpp
            src/common/api/HttpRequest.cpp
            )
    else()
        message(FATAL_ERROR "microhttpd NOT found: use `-DWITH_HTTPD=OFF` to build without http deamon support")
    endif()
else()
    set(HTTPD_SOURCES "")
    set(MHD_LIBRARY "")
    add_definitions(/DXMRIG_NO_HTTPD)
    add_definitions(/DXMRIG_NO_API)
endif()

if (WITH_TLS)
    find_package(OpenSSL)

    add_definitions(/DCPPHTTPLIB_OPENSSL_SUPPORT)

    if (OPENSSL_FOUND)
        include_directories(${OPENSSL_INCLUDE_DIR})
        set(SOURCES_SSL_TLS src/common/net/Tls.cpp)
    else()
        message(FATAL_ERROR "OpenSSL NOT found: use `-DWITH_TLS=OFF` to build without TLS support")
    endif()
else()
    add_definitions(/DXMRIG_NO_TLS)
endif()

if (WITH_CC_CLIENT)
    set(SOURCES_CC_CLIENT
            src/cc/CCClient.cpp)
endif()

if (WITH_CC_CLIENT)
    set(SOURCES_CC_COMMON
            src/cc/ControlCommand.cpp
            src/cc/ClientStatus.cpp
            src/cc/GPUInfo.cpp)
else()
    add_definitions(/DXMRIG_NO_CC)
endif()

include_directories(src)
include_directories(src/3rdparty)
include_directories(${UV_INCLUDE_DIR})

if (WITH_TLS)
    add_library(xmrig_tls STATIC ${SOURCES_SSL_TLS})
endif (WITH_TLS)

if (BUILD_STATIC)
    set(CMAKE_EXE_LINKER_FLAGS " -static")
endif()

if (WITH_DEBUG_LOG)
    add_definitions(/DAPP_DEBUG)
endif()
add_executable(xmrigMiner-amd ${HEADERS} ${SOURCES_CC_COMMON} ${SOURCES} ${SOURCES_COMMON} ${SOURCES_OS} ${SOURCES_CPUID} ${HEADERS_CRYPTO} ${SOURCES_CRYPTO} ${SOURCES_SYSLOG} ${HTTPD_SOURCES} ${SOURCES_CC_CLIENT} ${CN_GPU_SOURCES} ${XMRIG_ASM_SOURCES})
set_target_properties(xmrigMiner-amd PROPERTIES OUTPUT_NAME ${MINER_EXECUTABLE_NAME})
target_link_libraries(xmrigMiner-amd ${UV_LIBRARIES} ${XMRIG_ASM_LIBRARY} ${MHD_LIBRARY} ${EXTRA_LIBS} ${LIBS} ${OpenCL_LIBRARY})

if (WITH_TLS)
    target_link_libraries(xmrigMiner-amd xmrig_tls ${OPENSSL_LIBRARIES} ${EXTRA_LIBS})
endif (WITH_TLS)

add_executable(xmrigDaemon-amd src/cc/XMRigd.cpp res/app.rc)
set_target_properties(xmrigDaemon-amd PROPERTIES OUTPUT_NAME ${DAEMON_EXECUTABLE_NAME})
