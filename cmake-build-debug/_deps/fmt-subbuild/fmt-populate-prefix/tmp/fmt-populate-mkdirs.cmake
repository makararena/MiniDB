# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/ivanivsnov/CLionProjects/DatabaseProject/cmake-build-debug/_deps/fmt-src"
  "/Users/ivanivsnov/CLionProjects/DatabaseProject/cmake-build-debug/_deps/fmt-build"
  "/Users/ivanivsnov/CLionProjects/DatabaseProject/cmake-build-debug/_deps/fmt-subbuild/fmt-populate-prefix"
  "/Users/ivanivsnov/CLionProjects/DatabaseProject/cmake-build-debug/_deps/fmt-subbuild/fmt-populate-prefix/tmp"
  "/Users/ivanivsnov/CLionProjects/DatabaseProject/cmake-build-debug/_deps/fmt-subbuild/fmt-populate-prefix/src/fmt-populate-stamp"
  "/Users/ivanivsnov/CLionProjects/DatabaseProject/cmake-build-debug/_deps/fmt-subbuild/fmt-populate-prefix/src"
  "/Users/ivanivsnov/CLionProjects/DatabaseProject/cmake-build-debug/_deps/fmt-subbuild/fmt-populate-prefix/src/fmt-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/ivanivsnov/CLionProjects/DatabaseProject/cmake-build-debug/_deps/fmt-subbuild/fmt-populate-prefix/src/fmt-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/ivanivsnov/CLionProjects/DatabaseProject/cmake-build-debug/_deps/fmt-subbuild/fmt-populate-prefix/src/fmt-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
