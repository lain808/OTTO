set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE headers ${OTTO_SOURCE_DIR}/src/*.hpp)
file(GLOB_RECURSE sources ${OTTO_SOURCE_DIR}/src/*.cpp)

find_package (Threads)
# Library
add_library(otto ${sources})
target_link_libraries(otto PUBLIC ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})
target_include_directories(otto PUBLIC ./)

# Executable
add_executable(otto_exec ${OTTO_SOURCE_DIR}/boards/${OTTO_BOARD}/src/main.cpp)
target_link_libraries(otto_exec PUBLIC otto)
set_target_properties(otto_exec PROPERTIES OUTPUT_NAME otto)

add_subdirectory(${OTTO_EXTERNAL_DIR} ${OTTO_BINARY_DIR}/external)

# This updates configurations and includes board specific files
otto_include_board(${OTTO_BOARD})
otto_add_definitions(otto)
otto_add_definitions(otto_exec)

if (NOT OTTO_USE_LIBCXX)
  target_link_libraries(otto PUBLIC atomic)
endif()

target_link_libraries(otto PUBLIC external)
target_link_libraries(otto PUBLIC nanovg)
target_link_libraries(otto PUBLIC GSL)
target_link_libraries(otto PUBLIC fmt)
target_link_libraries(otto PUBLIC gamma)
target_link_libraries(otto PUBLIC foonathan_array)
target_link_libraries(otto PUBLIC foonathan_type_safe)
target_link_libraries(otto PUBLIC choreograph)
target_link_libraries(otto PUBLIC BFG::Lyra)

include("${OTTO_SOURCE_DIR}/cmake/doctest_force_link_static_lib_in_target.cmake")
doctest_force_link_static_lib_in_target(otto_exec otto)

# Enable warnings for local code
if(MSVC)
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -Wno-pedantic -Wno-c++14-compat -Wno-psabi -Wno-unknown-pragmas -Wno-vla -Wno-sign-compare")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -Wpedantic \
-Wno-c++11-compat -Wno-c++98-compat -Wno-c++14-compat -Wc++17-compat \
-Wno-shadow-field-in-constructor \
-Wno-documentation-unknown-command \
-Wno-unknown-pragmas \
-Wno-missing-braces \
-Wno-vla-extension \
-Wno-c99-extensions \
")
endif()

# Docs  
if (OTTO_BUILD_DOCS)    
# check if Doxygen is installed
find_package(Doxygen)
if (DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${OTTO_SOURCE_DIR}/doxygen/Doxyfile.in)
    set(DOXYGEN_OUT ${OTTO_BINARY_DIR}/doxygen/Doxyfile)
        
    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    # note the option ALL which allows to build the docs together with the application
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${OTTO_SOURCE_DIR}/doxygen
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate the doxygen documentation")
endif (DOXYGEN_FOUND)# check if Doxygen is installed
endif()
