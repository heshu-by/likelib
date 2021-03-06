add_subdirectory(evmc)

set(VM_HEADERS
        error.hpp
        vm.hpp
        messages.hpp
        tools.hpp
        )

set(VM_SOURCES
        vm.cpp
        messages.cpp
        tools.cpp
        )

set(EVMC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/evmc/include)

add_library(vm STATIC ${VM_HEADERS} ${VM_SOURCES})
target_include_directories(vm PUBLIC $<BUILD_INTERFACE:${EVMC_INCLUDE_DIR}>$<INSTALL_INTERFACE:include>)
target_link_libraries(vm loader OpenSSL::SSL)

# copy evm libs
file(GLOB EVM_LIB ${CONAN_BIN_DIRS_EVMONE}/*evmone*)
file(COPY ${EVM_LIB} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/)

# copy python files
file(GLOB PYTHON_FILES ${CMAKE_CURRENT_LIST_DIR}/python/*.py)
file(COPY ${PYTHON_FILES} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/)
