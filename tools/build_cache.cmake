# Project-owned cache policy; copied clean source trees explicitly share their
# caller's cache without depending on any cached object for correctness.
set(WIN16STYLE_CACHE_ROOT "${CMAKE_SOURCE_DIR}/build/.ccache" CACHE PATH "Persistent project ccache directory")
get_filename_component(WIN16STYLE_CACHE_ROOT "${WIN16STYLE_CACHE_ROOT}" ABSOLUTE)
if(IS_SYMLINK "${WIN16STYLE_CACHE_ROOT}" OR
   (EXISTS "${WIN16STYLE_CACHE_ROOT}" AND NOT IS_DIRECTORY "${WIN16STYLE_CACHE_ROOT}"))
  message(FATAL_ERROR "ccache root must be a real directory")
endif()
if(CMAKE_C_COMPILER_LAUNCHER)
  list(GET CMAKE_C_COMPILER_LAUNCHER 0 cache_launcher)
  get_filename_component(cache_launcher_name "${cache_launcher}" NAME)
  if(cache_launcher_name MATCHES "^ccache([.]exe)?$")
    set(CMAKE_C_COMPILER_LAUNCHER "${CMAKE_COMMAND}" -E env
      "CCACHE_DIR=${WIN16STYLE_CACHE_ROOT}"
      "CCACHE_TEMPDIR=${WIN16STYLE_CACHE_ROOT}/tmp"
      "CCACHE_MAXSIZE=5G" "CCACHE_BASEDIR=${CMAKE_SOURCE_DIR}"
      "${cache_launcher}")
  endif()
endif()
