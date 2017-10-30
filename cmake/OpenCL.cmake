# try to find AMD OpenCL before NVIDIA OpenCL
find_path(OpenCL_INCLUDE_DIR
    NAMES
        CL/cl.h
        OpenCL/cl.h
    NO_DEFAULT_PATH
    PATHS
        ENV "OpenCL_ROOT"
        ENV AMDAPPSDKROOT
        ENV ATISTREAMSDKROOT
        ENV "PROGRAMFILES(X86)"
    PATH_SUFFIXES
        include
        OpenCL/common/inc
        "AMD APP/include")

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    find_library(OpenCL_LIBRARY
    NAMES 
        OpenCL
        OpenCL.lib
    NO_DEFAULT_PATH
    PATHS
        ENV "OpenCL_ROOT"
        ENV AMDAPPSDKROOT
        ENV ATISTREAMSDKROOT
        ENV "PROGRAMFILES(X86)"
    PATH_SUFFIXES
        "AMD APP/lib/x86_64"
        lib/x86_64
        lib/x64
        OpenCL/common/lib/x64)
else()
    find_library(OpenCL_LIBRARY
    NAMES 
        OpenCL
        OpenCL.lib
    NO_DEFAULT_PATH
    PATHS
        ENV "OpenCL_ROOT"
        ENV AMDAPPSDKROOT
        ENV ATISTREAMSDKROOT
        ENV "PROGRAMFILES(X86)"
    PATH_SUFFIXES
        "AMD APP/lib/x86"
        lib/x86
        OpenCL/common/lib/x86)
endif()

# find package will use the previews searched path variables
find_package(OpenCL)
if(OpenCL_FOUND)
    include_directories(SYSTEM ${OpenCL_INCLUDE_DIRS})
    #set(LIBS ${LIBS} ${OpenCL_LIBRARY})
    link_directories(${OpenCL_LIBRARY})
endif()
