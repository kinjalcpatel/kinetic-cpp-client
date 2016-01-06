lib_LTLIBRARIES = libkinetic-cpp-client.la

libkinetic_cpp_client_includedir = $(includedir)/kinetic
libkinetic_cpp_client_include_HEADERS = \
$(top_srcdir)/include/kinetic/acls.h \
$(top_srcdir)/include/kinetic/kinetic_status.h \
$(top_srcdir)/include/kinetic/blocking_kinetic_connection.h \
$(top_srcdir)/include/kinetic/message_stream.h \
$(top_srcdir)/include/kinetic/blocking_kinetic_connection_interface.h \
$(top_srcdir)/include/kinetic/mock_incoming_value.h \
$(top_srcdir)/include/kinetic/byte_stream.h \
$(top_srcdir)/include/kinetic/mock_message_stream.h \
$(top_srcdir)/include/kinetic/common.h \
$(top_srcdir)/include/kinetic/nonblocking_kinetic_connection.h \
$(top_srcdir)/include/kinetic/connection_options.h \
$(top_srcdir)/include/kinetic/nonblocking_kinetic_connection_interface.h \
$(top_srcdir)/include/kinetic/drive_log.h \
$(top_srcdir)/include/kinetic/nonblocking_packet_service_interface.h \
$(top_srcdir)/include/kinetic/hmac_provider.h \
$(top_srcdir)/include/kinetic/outgoing_value.h \
$(top_srcdir)/include/kinetic/incoming_value.h \
$(top_srcdir)/include/kinetic/reader_writer.h \
$(top_srcdir)/include/kinetic/key_range_iterator.h \
$(top_srcdir)/include/kinetic/status_code.h \
$(top_srcdir)/include/kinetic/kinetic_connection_factory.h \
$(top_srcdir)/include/kinetic/status.h \
$(top_srcdir)/include/kinetic/kinetic_connection.h \
$(top_srcdir)/include/kinetic/threadsafe_blocking_kinetic_connection.h \
$(top_srcdir)/include/kinetic/kinetic.h \
$(top_srcdir)/include/kinetic/threadsafe_nonblocking_connection.h \
$(top_srcdir)/include/kinetic/kinetic_record.h


libkinetic_cpp_client_la_SOURCES = \
	blocking_kinetic_connection.cc \
	byte_stream.cc \
	nonblocking_packet_service.cc \
	hmac_provider.cc \
	nonblocking_packet_writer_factory.cc \
	incoming_string_value.cc \
	nonblocking_string.cc \
	key_range_iterator.cc \
	kinetic_connection_factory.cc \
	outgoing_string_value.cc \
	reader_writer.cc \
	message_stream.cc \
	socket_wrapper.cc \
	nonblocking_kinetic_connection.cc \
	nonblocking_packet.cc \
	status_code.cc \
	nonblocking_packet_receiver.cc \
	threadsafe_blocking_kinetic_connection.cc \
	threadsafe_nonblocking_kinetic_connection.cc \
	nonblocking_packet_sender.cc

libkinetic_cpp_client_la_CXXFLAGS = \
	-I$(top_srcdir)/src/main -I$(top_srcdir)/include

libkinetic_cpp_client_la_LDFLAGS = @LDFLAGS@ -version-info @SO_VERSION@

distclean-local:
	-rm -rf Makefile Makefile.in
