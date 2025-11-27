# Huawei Ascend Platform Configuration
# For Ascend 310/310P/910 platforms with CANN (Compute Architecture for Neural Networks)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(ARCH "aarch64")
set(PLATFORM "aarch64")
set(TARGET_PLATFORM "aarch64_ascend" CACHE STRING "")

# Compiler flags optimized for Ascend ARM
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O2 -g -Wall -fopenmp" CACHE STRING "")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall -fopenmp -DNDEBUG" CACHE STRING "")

# Ascend CANN Toolkit paths
set(ASCEND_ROOT "/usr/local/Ascend" CACHE PATH "Ascend installation directory")
if(NOT EXISTS ${ASCEND_ROOT})
    set(ASCEND_ROOT "/usr/local/ascend")
endif()

if(EXISTS ${ASCEND_ROOT})
    message(STATUS "Ascend CANN found at: ${ASCEND_ROOT}")
    
    # CANN version detection
    if(EXISTS "${ASCEND_ROOT}/latest")
        set(ASCEND_CANN_PATH "${ASCEND_ROOT}/latest")
    elseif(EXISTS "${ASCEND_ROOT}/ascend-toolkit/latest")
        set(ASCEND_CANN_PATH "${ASCEND_ROOT}/ascend-toolkit/latest")
    else()
        message(WARNING "CANN toolkit not found, please check Ascend installation")
    endif()
    
    if(DEFINED ASCEND_CANN_PATH)
        # ACL (Ascend Computing Language) runtime
        include_directories(${ASCEND_CANN_PATH}/include)
        link_directories(${ASCEND_CANN_PATH}/lib64)
        
        # AscendCL libraries
        set(ASCENDCL_LIBRARIES
            ascendcl
            acl_op_compiler
            acl_tdt_channel
            ge_runner
            graph
            runtime
        )
        
        add_definitions(-DENABLE_ASCENDCL)
        add_definitions(-DASCEND_PLATFORM)
        message(STATUS "AscendCL support enabled")
    endif()
else()
    message(WARNING "Ascend installation not found at ${ASCEND_ROOT}")
endif()

# OpenCV
find_package(OpenCV REQUIRED)
message(STATUS "OpenCV version: ${OpenCV_VERSION}")

# Additional Ascend environment variables (for runtime)
message(STATUS "")
message(STATUS "========================================")
message(STATUS "Ascend Platform Configuration")
message(STATUS "========================================")
message(STATUS "Please ensure the following environment variables are set:")
message(STATUS "  export ASCEND_HOME=${ASCEND_ROOT}")
message(STATUS "  export LD_LIBRARY_PATH=\${ASCEND_HOME}/lib64:\$LD_LIBRARY_PATH")
message(STATUS "  export PYTHONPATH=\${ASCEND_HOME}/python/site-packages:\$PYTHONPATH")
message(STATUS "========================================")
message(STATUS "")
