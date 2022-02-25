
add_subdirectory(qcustomplot)


# Build options
option(ENABLE_SAMPLES "" OFF)
option(ENABLE_OPENMP "" OFF)
option(ENABLE_WIDE_CHAR "" ON)
option(BUILD_SHARED_LIBS "" OFF)

add_subdirectory(muparser)
