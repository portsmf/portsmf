add_executable(smftest test.cpp)
target_link_libraries(smftest PortSMF)
add_test(NAME smftest COMMAND smftest)
set_tests_properties(smftest PROPERTIES
  ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_RUNTIME_DLL_DIRS:smftest>"
)
