SC_BEGIN_PACKAGE(SEEDLINK)

INCLUDE(CheckFunctionExists)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-incompatible-pointer-types")

SUBDIRS(libs apps plugins templates)
