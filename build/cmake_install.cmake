# Install script for directory: /home/sidhub/Desktop/ietf/libyang_meher

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/local/share/yang/modules/libyang/")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  file(INSTALL DESTINATION "/usr/local/share/yang/modules/libyang" TYPE DIRECTORY FILES "/home/sidhub/Desktop/ietf/libyang_meher/models/" FILES_MATCHING REGEX "/[^/]*\\.yang$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libyang.so.3.9.1"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libyang.so.3"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/sidhub/Desktop/ietf/libyang_meher/build/libyang.so.3.9.1"
    "/home/sidhub/Desktop/ietf/libyang_meher/build/libyang.so.3"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libyang.so.3.9.1"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libyang.so.3"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libyang.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libyang.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libyang.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/sidhub/Desktop/ietf/libyang_meher/build/libyang.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libyang.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libyang.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libyang.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/libyang" TYPE FILE FILES
    "/home/sidhub/Desktop/ietf/libyang_meher/src/context.h"
    "/home/sidhub/Desktop/ietf/libyang_meher/src/hash_table.h"
    "/home/sidhub/Desktop/ietf/libyang_meher/src/dict.h"
    "/home/sidhub/Desktop/ietf/libyang_meher/src/in.h"
    "/home/sidhub/Desktop/ietf/libyang_meher/src/libyang.h"
    "/home/sidhub/Desktop/ietf/libyang_meher/src/log.h"
    "/home/sidhub/Desktop/ietf/libyang_meher/src/out.h"
    "/home/sidhub/Desktop/ietf/libyang_meher/src/parser_data.h"
    "/home/sidhub/Desktop/ietf/libyang_meher/src/parser_schema.h"
    "/home/sidhub/Desktop/ietf/libyang_meher/src/plugins.h"
    "/home/sidhub/Desktop/ietf/libyang_meher/src/plugins_exts.h"
    "/home/sidhub/Desktop/ietf/libyang_meher/src/plugins_exts/metadata.h"
    "/home/sidhub/Desktop/ietf/libyang_meher/src/plugins_types.h"
    "/home/sidhub/Desktop/ietf/libyang_meher/src/printer_data.h"
    "/home/sidhub/Desktop/ietf/libyang_meher/src/printer_schema.h"
    "/home/sidhub/Desktop/ietf/libyang_meher/src/set.h"
    "/home/sidhub/Desktop/ietf/libyang_meher/src/tree.h"
    "/home/sidhub/Desktop/ietf/libyang_meher/src/tree_data.h"
    "/home/sidhub/Desktop/ietf/libyang_meher/src/tree_edit.h"
    "/home/sidhub/Desktop/ietf/libyang_meher/src/tree_schema.h"
    "/home/sidhub/Desktop/ietf/libyang_meher/src/parser_cbor.h"
    "/home/sidhub/Desktop/ietf/libyang_meher/build/libyang/version.h"
    "/home/sidhub/Desktop/ietf/libyang_meher/build/libyang/ly_config.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/sidhub/Desktop/ietf/libyang_meher/build/libyang.pc")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/sidhub/Desktop/ietf/libyang_meher/build/tools/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/sidhub/Desktop/ietf/libyang_meher/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
