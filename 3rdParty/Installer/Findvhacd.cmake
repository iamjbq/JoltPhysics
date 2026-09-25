
if (TARGET 3rdParty::vhacd)
  return()
endif()

add_library(3rdParty::vhacd IMPORTED INTERFACE GLOBAL)

message(STATUS "Jolt Physics uses v-hacd v4.1.0 (BSD-3-Clause) https://github.com/kmammou/v-hacd.git")
set(vhacd_FOUND TRUE)
