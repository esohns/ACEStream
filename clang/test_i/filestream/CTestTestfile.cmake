# CMake generated Testfile for 
# Source directory: /mnt/win_d/projects/ACEStream/test_i/filestream
# Build directory: /mnt/win_d/projects/ACEStream/clang/test_i/filestream
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(filestream_source "/mnt/win_d/projects/ACEStream/clang/test_i/filestream/filesource" "-l" "-t")
set_tests_properties(filestream_source PROPERTIES  ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;/mnt/win_d/projects/ACEStream\\..\\Common\\cmake\\src\\Debug;/mnt/win_d/projects/ACEStream\\..\\Common\\cmake\\src\\ui\\Debug;/mnt/win_d/projects/ACEStream\\cmake\\src\\Debug;/mnt/win_d/projects/ACEStream\\cmake\\src\\modules\\dev\\Debug;/mnt/win_d/projects/ACEStream\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%" WORKING_DIRECTORY "/mnt/win_d/projects/ACEStream")
add_test(filetarget_test "/mnt/win_d/projects/ACEStream/clang/filetarget" "-l" "-t")
set_tests_properties(filetarget_test PROPERTIES  ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;/mnt/win_d/projects/ACEStream\\..\\Common\\cmake\\src\\Debug;/mnt/win_d/projects/ACEStream\\..\\Common\\cmake\\src\\ui\\Debug;/mnt/win_d/projects/ACEStream\\cmake\\src\\Debug;/mnt/win_d/projects/ACEStream\\cmake\\src\\modules\\dev\\Debug;/mnt/win_d/projects/ACEStream\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%" WORKING_DIRECTORY "/mnt/win_d/projects/ACEStream")
