# where to install python binding (default: user local)
if(${PYTHON_INSTALL_LOCAL})
  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -m site --user-site
    OUTPUT_VARIABLE PYTHON_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE)
else(${PYTHON_INSTALL_LOCAL})
  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} -c "from distutils.sysconfig import get_python_lib; print(get_python_lib())"
    OUTPUT_VARIABLE PYTHON_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE)
endif(${PYTHON_INSTALL_LOCAL})


# Get numpy header directory
execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "from numpy import get_include; print(get_include())"
  OUTPUT_VARIABLE NUMPY_HEADER_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
INCLUDE_DIRECTORIES(${NUMPY_HEADER_DIR})

SET(CMAKE_SWIG_FLAGS "")

SET_SOURCE_FILES_PROPERTIES(stochbb.i PROPERTIES CPLUSPLUS ON)
SET_SOURCE_FILES_PROPERTIES(stochbb.i PROPERTIES SWIG_FLAGS "-includeall")

SWIG_ADD_MODULE(stochbb python stochbb.i)
TARGET_LINK_LIBRARIES(_stochbb ${PYTHON_LIBRARIES} ${StochBB_LIBRARIES})

install(TARGETS _stochbb DESTINATION ${PYTHON_SITE_PACKAGES})
install(FILES ${CMAKE_BINARY_DIR}/python/stochbb.py DESTINATION ${PYTHON_SITE_PACKAGES})
