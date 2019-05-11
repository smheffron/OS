cmake_minimum_required (VERSION 2.8)
project(assignment4)

set(CMAKE_C_FLAGS "-std=c11 -Wall -Wextra -Wshadow -Werror")
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra -Wshadow -Werror")

# Read some documentation, do some googling, refer to things we've given you
# cmake is popular, so someone on the internet has had your problem before


include_directories("${PROJECT_SOURCE_DIR}/include")

# build a dynamic library called libblock_store.so
add_library(block_store SHARED src/block_store.c include/block_store.h src/bitmap.c include/bitmap.h)

# note that the prefix lib will be automatically added in the filename.


# make an executable
add_executable(${PROJECT_NAME}_test test/tests.cpp)

# Enable grad/bonus tests by setting the variable to 1
target_compile_definitions(${PROJECT_NAME}_test PRIVATE GRAD_TESTS=1)

# link our library to the test file
target_link_libraries(${PROJECT_NAME}_test gtest pthread block_store)