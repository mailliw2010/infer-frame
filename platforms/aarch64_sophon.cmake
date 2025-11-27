# Sophon BM1684X Platform Configuration
# For Sophon BM1684X/BM1688 edge AI accelerators

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(ARCH "aarch64")
set(PLATFORM "aarch64")
set(TARGET_PLATFORM "aarch64_sophon" CACHE STRING "")

# Compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O2 -g -Wall -fopenmp" CACHE STRING "")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall -fopenmp -DNDEBUG" CACHE STRING "")

# Sophon SDK
option(ENABLE_SOPHON "Enable Sophon BMRuntime support" ON)
if(ENABLE_SOPHON)
    set(SOPHON_SDK_ROOT "/opt/sophon" CACHE PATH "Sophon SDK installation directory")
    
    if(EXISTS "${SOPHON_SDK_ROOT}")
        # BMLib (basic library)
        include_directories(${SOPHON_SDK_ROOT}/sophon-ffmpeg-latest/include)
        include_directories(${SOPHON_SDK_ROOT}/sophon-opencv-latest/include)
        include_directories(${SOPHON_SDK_ROOT}/libsophon-current/include)
        
        link_directories(${SOPHON_SDK_ROOT}/sophon-ffmpeg-latest/lib)
        link_directories(${SOPHON_SDK_ROOT}/sophon-opencv-latest/lib)
        link_directories(${SOPHON_SDK_ROOT}/libsophon-current/lib)
        
        # BMRuntime for inference
        set(SOPHON_LIBRARIES bmrt bmlib)
        add_definitions(-DENABLE_SOPHON)
        add_definitions(-DSOPHON_PLATFORM)
        message(STATUS "Sophon SDK found at ${SOPHON_SDK_ROOT}")
    else()
        message(WARNING "Sophon SDK not found at ${SOPHON_SDK_ROOT}")
    endif()
endif()

# Sophon SAIL (Sophon AI Library) - optional high-level API
option(ENABLE_SAIL "Enable Sophon SAIL library" OFF)
if(ENABLE_SAIL)
    find_library(SAIL_LIBRARY sail PATHS ${SOPHON_SDK_ROOT}/sophon-sail/lib)
    if(SAIL_LIBRARY)
        include_directories(${SOPHON_SDK_ROOT}/sophon-sail/include)
        set(SAIL_LIBRARIES ${SAIL_LIBRARY})
        add_definitions(-DENABLE_SAIL)
        message(STATUS "Sophon SAIL library found")
    endif()
endif()

# Sophon hardware codec
option(ENABLE_SOPHON_CODEC "Enable Sophon hardware codec" ON)
if(ENABLE_SOPHON_CODEC)
    add_definitions(-DENABLE_SOPHON_CODEC)
    message(STATUS "Sophon hardware codec enabled")
endif()
