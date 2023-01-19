cmake_minimum_required(VERSION 3.8)
project(spdr CXX)

set(PROJECT_VERSION 0.2.0)
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 2)
set(PROJECT_VERSION_PATCH 0)

option(ENABLE_UNIT_TESTS "Enable building unit tests." OFF)
option(ENABLE_EXAMPLES   "Enable building examples." OFF)

set(CMAKE_SHARED_LIBRARY_PREFIX)
set(CMAKE_STATIC_LIBRARY_PREFIX)

if(ENABLE_UNIT_TESTS)
  find_package(GTest CONFIG REQUIRED)
endif()

find_package(c9y CONFIG REQUIRED)

set(HEADERS
  spdr/config.h
  spdr/spdr.h
  spdr/IpAddress.h
  spdr/UdpSocket.h
  spdr/pack.h
  spdr/Node.h
)

set(SOURCES
  spdr/pch.cpp
  spdr/spdr.h
  spdr/IpAddress.cpp
  spdr/UdpSocket.cpp
  spdr/pack.cpp
  spdr/Node.cpp
)

add_library(spdr ${SOURCES})

if (WIN32)
  target_link_libraries(spdr PRIVATE c9y::c9y ws2_32.lib)
else()
  target_link_libraries(spdr PRIVATE c9y::c9y)
endif()

set_target_properties(spdr PROPERTIES
    CXX_STANDARD 20
    PUBLIC_HEADER "${HEADERS}"
)

if(ENABLE_UNIT_TESTS)

  enable_testing()

  set(TEST_SOURCES
    spdr-test/main.cpp
    spdr-test/pch.cpp
    spdr-test/test_node.cpp
    spdr-test/test_pack.cpp
    spdr-test/test_udp_socket.cpp
  )
  
  include_directories(.)
  add_executable(spdr-test ${TEST_SOURCES})
  set_target_properties(spdr-test PROPERTIES
    CXX_STANDARD 20
  )
  target_link_libraries(spdr-test PRIVATE spdr c9y::c9y GTest::gtest)
  add_test(spdr-test spdr-test)

endif()

if(ENABLE_EXAMPLES)
  set(CHAT_SOURCES
    examples/chat/pch.cpp
    examples/chat/main.cpp
    examples/chat/ChatServer.cpp
    examples/chat/ChatClient.cpp
  )
  
  include_directories(.)
  add_executable(spdr-chat ${CHAT_SOURCES})
  set_target_properties(spdr-chat PROPERTIES
    CXX_STANDARD 20
  )
  target_link_libraries(spdr-chat PRIVATE spdr c9y::c9y)

endif()

# install
install(
  TARGETS spdr
  EXPORT spdrTargets
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include/spdr
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file("${PROJECT_BINARY_DIR}/spdrConfigVersion.cmake"
                                 COMPATIBILITY SameMajorVersion)

configure_package_config_file(${PROJECT_SOURCE_DIR}/spdrConfig.cmake.in
spdrConfig.cmake
                                INSTALL_DESTINATION share/spdr)

install(FILES
        "${PROJECT_BINARY_DIR}/spdrConfig.cmake"
        "${PROJECT_BINARY_DIR}/spdrConfigVersion.cmake"
        DESTINATION share/spdr
)

install(EXPORT spdrTargets
        NAMESPACE spdr::
        DESTINATION share/spdr
)