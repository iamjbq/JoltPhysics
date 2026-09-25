
if (TARGET 3rdParty::vhacd)
  return()
endif()

o3de_fetch_content(vhacd
    VERSION "v4.1.0"
    LICENSE "BSD-3-Clause"
    URL "https://github.com/kmammou/v-hacd/archive/refs/tags/v4.1.0.tar.gz"
    URL_HASH "9fe895cd10ec995d2171b11bde97aaaa221b418a3aaed0f5d9a068ae057d626b"
    GIT "https://github.com/kmammou/v-hacd.git"
    GIT_HASH "22ec20a7f8ea221ab600df869369b9c6a258cb10"
)

FetchContent_MakeAvailable(vhacd)

FetchContent_GetProperties(vhacd SOURCE_DIR V_HACD_SOURCE_DIR)
add_library(3rdParty::vhacd IMPORTED INTERFACE GLOBAL ${V_HACD_SOURCE_DIR}/include/VHACD.h)
ly_target_include_system_directories(TARGET 3rdParty::vhacd INTERFACE ${V_HACD_SOURCE_DIR}/include)

ly_install(FILES ${CMAKE_CURRENT_LIST_DIR}/Installer/Findvhacd.cmake DESTINATION cmake/3rdParty)
set(vhacd_FOUND TRUE)
