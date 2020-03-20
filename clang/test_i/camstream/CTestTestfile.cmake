# CMake generated Testfile for 
# Source directory: /mnt/win_d/projects/ACEStream/test_i/camstream
# Build directory: /mnt/win_d/projects/ACEStream/clang/test_i/camstream
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(camsource_test "/mnt/win_d/projects/ACEStream/clang/camsource" "-l" "-t")
set_tests_properties(camsource_test PROPERTIES  ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;/mnt/win_d/projects/ACEStream\\..\\Common\\cmake\\src\\Debug;/mnt/win_d/projects/ACEStream\\..\\Common\\cmake\\src\\ui\\Debug;/mnt/win_d/projects/ACEStream\\cmake\\src\\Debug;/mnt/win_d/projects/ACEStream\\cmake\\src\\modules\\dev\\Debug;/mnt/win_d/projects/ACEStream\\..\\ACENetwork\\cmake\\src\\Debug;%PATH%" WORKING_DIRECTORY "/mnt/win_d/projects/ACEStream")
add_test(camtarget_test "/mnt/win_d/projects/ACEStream/clang" "-l" "-t")
set_tests_properties(camtarget_test PROPERTIES  ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;/mnt/win_d/projects/ACEStream\\..\\Common\\cmake\\src\\Debug;/mnt/win_d/projects/ACEStream\\..\\Common\\cmake\\src\\ui\\Debug;/mnt/win_d/projects/ACEStream\\cmake\\src\\Debug;/mnt/win_d/projects/ACEStream\\cmake\\src\\modules\\dev\\Debug;/mnt/win_d/projects/ACEStream\\..\\ACENetwork\\cmake\\src\\Debug;%PATH%" WORKING_DIRECTORY "/mnt/win_d/projects/ACEStream")
