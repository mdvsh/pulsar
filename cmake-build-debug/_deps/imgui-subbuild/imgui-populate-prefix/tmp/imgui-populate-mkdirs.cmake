# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/sharmadhavs/Downloads/winter-24/498/pulsar3/cmake-build-debug/_deps/imgui-src"
  "/Users/sharmadhavs/Downloads/winter-24/498/pulsar3/cmake-build-debug/_deps/imgui-build"
  "/Users/sharmadhavs/Downloads/winter-24/498/pulsar3/cmake-build-debug/_deps/imgui-subbuild/imgui-populate-prefix"
  "/Users/sharmadhavs/Downloads/winter-24/498/pulsar3/cmake-build-debug/_deps/imgui-subbuild/imgui-populate-prefix/tmp"
  "/Users/sharmadhavs/Downloads/winter-24/498/pulsar3/cmake-build-debug/_deps/imgui-subbuild/imgui-populate-prefix/src/imgui-populate-stamp"
  "/Users/sharmadhavs/Downloads/winter-24/498/pulsar3/cmake-build-debug/_deps/imgui-subbuild/imgui-populate-prefix/src"
  "/Users/sharmadhavs/Downloads/winter-24/498/pulsar3/cmake-build-debug/_deps/imgui-subbuild/imgui-populate-prefix/src/imgui-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/sharmadhavs/Downloads/winter-24/498/pulsar3/cmake-build-debug/_deps/imgui-subbuild/imgui-populate-prefix/src/imgui-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/sharmadhavs/Downloads/winter-24/498/pulsar3/cmake-build-debug/_deps/imgui-subbuild/imgui-populate-prefix/src/imgui-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
