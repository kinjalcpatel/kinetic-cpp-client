if ENABLE_TEST
check_PROGRAMS += kinetic_cpp_client_test

kinetic_cpp_client_test_SOURCES = \
	src/test/kinetic_cpp_client_test.cc \
	src/test/nonblocking_kinetic_connection_test.cc \
	src/test/nonblocking_packet_service_test.cc \
	src/test/nonblocking_packet_sender_test.cc \
	src/test/nonblocking_packet_receiver_test.cc \
	src/test/nonblocking_packet_test.cc \
	src/test/nonblocking_string_test.cc\
	src/test/hmac_provider_test.cc \
	src/test/message_stream_test.cc \
	src/test/string_value_test.cc

kinetic_cpp_client_test_LDADD = $(top_builddir)/src/main/libkinetic-cpp-client.la
kinetic_cpp_client_test_CPPFLAGS = \
	-I$(top_srcdir)/src/test/mock -I$(top_srcdir)/include \
	-I$(top_srcdir)/src/main
endif
