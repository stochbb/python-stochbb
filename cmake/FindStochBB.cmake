# - Find libstochbb
#
#  StochBB_INCLUDE_DIRS - where to find stochbb.hh
#  StochBB_LIBRARIES    - List of libraries when using libstochbb.
#  StochBB_FOUND        - True if libstochbb found.

if(StochBB_INCLUDE_DIRS)
  # Already in cache, be silent
  set(StochBB_FIND_QUIETLY TRUE)
endif(StochBB_INCLUDE_DIRS)

find_path(StochBB_INCLUDE_DIRS "stochbb/stochbb.hh")
find_library(StochBB_LIBRARIES NAMES stochbb)

# handle the QUIETLY and REQUIRED arguments and set STOCHBB_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(StochBB DEFAULT_MSG StochBB_LIBRARIES StochBB_INCLUDE_DIRS)

mark_as_advanced(StochBB_LIBRARIES StochBB_INCLUDE_DIRS)
