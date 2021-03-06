#
# Copyright(C) 2008 Doug Judd (Zvents, Inc.)
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.
#

set(Common_SRCS
Checksum.cc
CrashTest.cc
code_search_and_replace.cc
Config.cc
Error.cc
FileUtils.cc
InetAddr.cc
Init.cc
InteractiveCommand.cc
Logger.cc
Properties.cc
String.cc
System.cc
Time.cc
Usage.cc
Version.cc
md5.cc
)

add_library(HyperCommon ${Common_SRCS})
target_link_libraries(HyperCommon ${BOOST_LIBS} ${Log4cpp_LIBRARIES}
    ${ZLIB_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

# handy utils
add_executable(code_search_and_replace code_search_and_replace.cc)
target_link_libraries(code_search_and_replace HyperCommon)

# tests
add_executable(exception_test tests/exception_test.cc)
target_link_libraries(exception_test HyperCommon)

add_executable(logging_test tests/logging_test.cc)
target_link_libraries(logging_test HyperCommon)

# serialization tests
add_executable(sertest tests/sertest.cc)
target_link_libraries(sertest HyperCommon)

# macro expanded formatted sertest.cc for easy debugging
# sertest-x.cc is generated by gpp included in toplevel bin/gpp
#add_executable(sertestx tests/sertest-x.cc)
#target_link_libraries(sertestx HyperCommon)

add_test(Common-Exception exception_test)
add_test(Common-Logging logging_test)
add_test(Common-Serialization sertest)

set(VERSION_H ${HYPERTABLE_BINARY_DIR}/src/cc/Common/Version.h)

configure_file(Version.h.in ${VERSION_H})

file(GLOB HEADERS *.h)

install(FILES ${HEADERS} ${VERSION_H} DESTINATION ${VERSION}/include/Common)

install(TARGETS HyperCommon
        RUNTIME DESTINATION ${VERSION}/bin
        LIBRARY DESTINATION ${VERSION}/lib
        ARCHIVE DESTINATION ${VERSION}/lib)

install(TARGETS code_search_and_replace
	RUNTIME DESTINATION ${VERSION}/bin)
