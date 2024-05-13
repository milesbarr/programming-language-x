file(GLOB SRCS *.h *.c)
add_executable(${CMAKE_PROJECT_NAME}_test ${SRCS})
target_link_libraries(${CMAKE_PROJECT_NAME}_test PRIVATE ${CMAKE_PROJECT_NAME}_lib)
target_compile_features(${CMAKE_PROJECT_NAME}_test PRIVATE cxx_std_20)
# target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
#   $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
#   $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
# )
