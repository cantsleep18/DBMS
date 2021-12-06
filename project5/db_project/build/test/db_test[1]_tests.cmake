add_test( FileInitTest.HandlesInitialization /home/cantsleep/2021_ite2038_2018007429/project5/db_project/build/bin/db_test [==[--gtest_filter=FileInitTest.HandlesInitialization]==] --gtest_also_run_disabled_tests)
set_tests_properties( FileInitTest.HandlesInitialization PROPERTIES WORKING_DIRECTORY /home/cantsleep/2021_ite2038_2018007429/project5/db_project/build/test SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==])
add_test( FileTest.HandlesPageAllocation /home/cantsleep/2021_ite2038_2018007429/project5/db_project/build/bin/db_test [==[--gtest_filter=FileTest.HandlesPageAllocation]==] --gtest_also_run_disabled_tests)
set_tests_properties( FileTest.HandlesPageAllocation PROPERTIES WORKING_DIRECTORY /home/cantsleep/2021_ite2038_2018007429/project5/db_project/build/test SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==])
add_test( FileTest.CheckReadWriteOperation /home/cantsleep/2021_ite2038_2018007429/project5/db_project/build/bin/db_test [==[--gtest_filter=FileTest.CheckReadWriteOperation]==] --gtest_also_run_disabled_tests)
set_tests_properties( FileTest.CheckReadWriteOperation PROPERTIES WORKING_DIRECTORY /home/cantsleep/2021_ite2038_2018007429/project5/db_project/build/test SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==])
set( db_test_TESTS FileInitTest.HandlesInitialization FileTest.HandlesPageAllocation FileTest.CheckReadWriteOperation)
