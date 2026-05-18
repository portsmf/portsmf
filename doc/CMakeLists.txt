find_package(Doxygen)
if(Doxygen_FOUND)
  configure_file(Doxyfile.in Doxyfile @ONLY)
  doxygen_add_docs(
    doc
    COMMENT "Generating HTML documentation with Doxygen"
    CONFIG_FILE ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
  )
  install(
    FILES ${PROJECT_BINARY_DIR}/doc/html
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    COMPONENT doc
    OPTIONAL
  )
ENDIF()
