if ENABLE_TEST
check_PROGRAMS += kinetic_cpp_integration_test

kinetic_cpp_integration_test_SOURCES = \
    src/integration_test/delete_test.cc \
    src/integration_test/get_test.cc \
    src/integration_test/nonexistent_server_test.cc \
    src/integration_test/put_test.cc \
    src/integration_test/blocking_smoketest.cc \
    src/test/kinetic_cpp_client_test.cc

kinetic_cpp_integration_test_LDADD = $(top_builddir)/src/main/libkinetic-cpp-client.la
kinetic_cpp_integration_test_CPPFLAGS = \
	-I$(top_srcdir)/include -I$(top_srcdir)/src/main \
	-I$(top_srcdir)/src/test -I$(top_srcdir)/src/test/mock
endif
