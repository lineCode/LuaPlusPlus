project(UnitTest++)

file(GLOB_RECURSE SOURCE_FILES *.cpp *.h)

list(REMOVE_ITEM SOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/Win32/TimeHelpers.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Win32/TimeHelpers.h
)

add_library(${PROJECT_NAME} ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
