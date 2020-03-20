# CMake generated Testfile for 
# Source directory: /mnt/win_d/projects/ACEStream/test_u/audioeffect
# Build directory: /mnt/win_d/projects/ACEStream/clang/test_u/audioeffect
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(audioeffect_test "/mnt/win_d/projects/ACEStream/clang/audioeffect" "-l" "-t")
set_tests_properties(audioeffect_test PROPERTIES  ENVIRONMENT "PATH=D:\\projects\\ATCD\\ACE\\lib;D:\\projects\\gtk\\bin;D:\\projects\\libglade\\bin;D:\\projects\\libxml2-2.9.1\\.libs;G:\\software\\Development\\dll;/mnt/win_d/projects/ACEStream\\..\\Common\\cmake\\src\\Debug;/mnt/win_d/projects/ACEStream\\..\\Common\\cmake\\src\\ui\\Debug;/mnt/win_d/projects/ACEStream\\cmake\\src\\Debug;/mnt/win_d/projects/ACEStream\\cmake\\src\\modules\\dev\\Debug;/mnt/win_d/projects/ACEStream\\..\\libACENetwork\\cmake\\src\\Debug;%PATH%" WORKING_DIRECTORY "/mnt/win_d/projects/ACEStream")
