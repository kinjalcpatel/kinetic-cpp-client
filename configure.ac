#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])

m4_define([VESRION_STR],
m4_esyscmd([git describe --tags --abbrev=0 | tail -1 | tr -d '\n']))
AC_INIT([kinetic-cpp-client], [VERSION_STR], [info@openkinetic.org])

AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])
AC_CONFIG_MACRO_DIR([m4])

AC_LANG([C++])
# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AM_PROG_AR
AM_PROG_LIBTOOL

# Update this value for every release!  (A:B:C will map to foo.so.(A-C).C.B)
# http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
AC_SUBST(SO_VERSION, [1:1:1])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stddef.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])
AC_CHECK_HEADER([kinetic/kinetic.pb.h], [], AC_MSG_ERROR(cannot find kinetic protocol headers))
AC_CHECK_HEADER([openssl/sha.h], [], AC_MSG_ERROR(cannot find openssl headers))
AC_CHECK_HEADER([google/protobuf/message.h], [], AC_MSG_ERROR(cannot find protobuf headers))
AC_CHECK_HEADER([glog/logging.h], [], AC_MSG_ERROR(cannot find glog headers))
AC_CHECK_HEADER([gtest/gtest.h], [], AC_MSG_ERROR(cannot find gtest headers))
AC_CHECK_HEADER([gmock/gmock.h], [], AC_MSG_ERROR(cannot find gmock headers))

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_FORK
AC_CHECK_FUNCS([acl memset select socket strerror])

AC_CHECK_LIB([protobuf], [main], [LIBS="$LIBS -lprotobuf"],
	AC_MSG_ERROR(libprotobuf is not installed))
AC_CHECK_LIB([crypto], [SHA1], [LIBS="$LIBS -lcrypto"],
        AC_MSG_ERROR(libcrypto is not installed))
AC_CHECK_LIB([ssl], [SSL_library_init], [LIBS="$LIBS -lssl"],
        AC_MSG_ERROR(libssl is not installed))
AC_CHECK_LIB([pthread], [main],[LIBS="$LIBS -lpthread"],
	AC_MSG_ERROR(echo "libpthread is not installed"))

LIBS="-lkinetic.pb $LIBS"
AC_LINK_IFELSE(
        [AC_LANG_PROGRAM([#include <kinetic/kinetic.pb.h>], [com::seagate::kinetic::proto::Command test; test.Clear()])],
        [HAVE_KINETIC=1],
        [AC_MSG_ERROR([libkinetic.pb is not installed.])])

AC_CHECK_LIB([gtest], [main], [LIBS="$LIBS -lgtest"],
	AC_MSG_ERROR(libgtest is not installed))

LIBS="$LIBS -lgflags"
AC_LINK_IFELSE(
        [AC_LANG_PROGRAM([#include "gflags/gflags.h"], [google::ShutDownCommandLineFlags()])],
        [HAVE_GFLAGS=1],
        [AC_MSG_ERROR([libgflags is not installed.])])

LIBS="$LIBS -lgmock"
AC_LINK_IFELSE(
	[AC_LANG_PROGRAM([#include <gmock/gmock.h>], [testing::Cardinality dummy])],
	[HAVE_GMOCK=1],
	[AC_MSG_ERROR([libgmock is not installed.])])

LIBS="$LIBS -lglog"
AC_LINK_IFELSE(
        [AC_LANG_PROGRAM([#include "glog/logging.h"], [google::ShutdownGoogleLogging()])],
        [HAVE_GLOG=1],
        [AC_MSG_ERROR([libglog is not installed.])])

SAVED_LIBS=$LIBS
AC_CHECK_LIB([unwind], [backtrace], [LIBS="$LIBS -lunwind"],
	AC_MSG_NOTICE([libunwind is not installed]))
LIBS=$SAVED_LIBS

CXXFLAGS="--std=c++0x -Wall -Wextra -Wno-unknown-warning-option \
        -Wno-unused-parameter -Wno-null-dereference -Wno-unused-local-typedefs \
        -DGTEST_USE_OWN_TR1_TUPLE=1 -fPIC ${CXXFLAGS}"

AC_ARG_ENABLE(test,
	[AS_HELP_STRING([--enable-test[[=ARG]]],[enable building system test targets [ARG=yes]])],
	enable_test=$enableval, enable_test=yes)
AM_CONDITIONAL([ENABLE_TEST], [test x$enable_test = xyes])

AC_CONFIG_FILES([Makefile src/main/Makefile])

AC_OUTPUT

AC_MSG_RESULT([
	$PACKAGE-$VERSION

	CXX:		${CXX}
	CXXCPP:		${CXXCPP}
	CXXFLAGS:	${CXXFLAGS}
	CPPFLAGS:	${CPPFLAGS}
	LDFLAGS:	${LDFLAGS}
	LIBS:		${LIBS}

	prefix:		${prefix}
	exec_prefix:	${exec_prefix}
	libdir:		${libdir}
	includedir:	${includedir}
	bindir:		${bindir}
])
