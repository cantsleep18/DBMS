if(EXISTS "/home/cantsleep/2021_ite2038_2018007429/project5/db_project/build/test/db_test[1]_tests.cmake")
  include("/home/cantsleep/2021_ite2038_2018007429/project5/db_project/build/test/db_test[1]_tests.cmake")
else()
  add_test(db_test_NOT_BUILT db_test_NOT_BUILT)
endif()
