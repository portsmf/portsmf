FIND_PACKAGE(Doxygen)

IF(DOXYGEN_FOUND OR Doxygen_FOUND)
  SET(DOXYGEN_INPUT_DIR ${PROJECT_SOURCE_DIR})
  SET(DOXYGEN_OUTPUT_DIR ${PROJECT_BINARY_DIR}/doc)

  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

  ADD_CUSTOM_TARGET(doc
    COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${DOXYGEN_INPUT_DIR}
    COMMENT "Generating documentation with Doxygen"
    SOURCES Doxyfile.in)

  # FIXME: The Github runner tries to install docs without actually
  # building them first. This is certainly a skill issue on my part.
  # The OPTIONAL here should be removed once I figure out how to make
  # the install step properly depend upon the doc custom target.
  INSTALL(FILES ${DOXYGEN_OUTPUT_DIR}/html
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    COMPONENT doc
    OPTIONAL)
ENDIF()
