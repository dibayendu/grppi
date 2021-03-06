# GOOGLE TEST----------------------------------------------------------------------------
option(GRPPI_UNIT_TEST_ENABLE "Activate unit tests" OFF)

if(GRPPI_UNIT_TEST_ENABLE)
  
  # Set policy to support newer version of CMake and GTest
  # We will remove this when minimum version of CMake for GrPPI is move to 3.3 or
  # higher
  if(POLICY CMP0057)
    cmake_policy(SET CMP0057 NEW)
  endif()

  find_package(GTest REQUIRED) 
  include_directories(${GTEST_INCLUDE_DIRS})

  # Unit testing should be made in debug mode
  set(CMAKE_BUILD_TYPE "Debug")

  if ( NOT (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang") )
    # Add coverage options
    add_compile_options(--coverage)
  endif()

  set(PROJECT_TEST_NAME utest_${PROJECT_NAME_STR})
  file(GLOB TEST_SRC_FILES
      ${CMAKE_CURRENT_SOURCE_DIR}/*cpp)

  add_executable(${PROJECT_TEST_NAME} ${TEST_SRC_FILES})
  target_link_libraries(${PROJECT_TEST_NAME} 
      ${GTEST_BOTH_LIBRARIES} 
      gcov
      ${TBB_LIBRARIES}
      ${CMAKE_THREAD_LIBS_INIT}
      ${Boost_LIBRARIES}
  )
  
  GTEST_ADD_TESTS(${PROJECT_TEST_NAME} "" ${TEST_SRC_FILES})

  # Coverage options
  if ( NOT (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang") )

    find_program(LCOV_CMD lcov)
    find_program(GENHTML_CMD genhtml)

    ADD_CUSTOM_TARGET(coverage
        ${LCOV_CMD} --directory . --zerocounters
        COMMAND ${PROJECT_TEST_NAME}
        COMMAND ${LCOV_CMD} --directory . --capture --output-file mycov.info
        COMMAND ${LCOV_CMD} --remove mycov.info '/usr/*' 'unit_tests/*' 
              --output-file mycov.info.cleaned
        COMMAND ${GENHTML_CMD} -o mycov mycov.info.cleaned --legend -s 
        COMMAND ${CMAKE_COMMAND} -E remove mycov.info mycov.info.cleaned
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Computing coverage and generating report"
    )
  endif()

endif(GRPPI_UNIT_TEST_ENABLE)
