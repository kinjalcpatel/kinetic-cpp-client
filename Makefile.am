ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST =

SUBDIRS = src/main

if ENABLE_TEST

check_PROGRAMS =
include src/test/Makefile.am
include src/integration_test/Makefile.am

# Rules for running unit and integration tests under Valgrind
test_valgrind: check
	valgrind --leak-check=full --show-reachable=yes --track-fds=yes --suppressions=$(top_srcdir)/valgrind_linux.supp $(top_srcdir)/kinetic_cpp_client_test

integration_test_valgrind: check
	valgrind --leak-check=full --show-reachable=yes --track-fds=yes --suppressions=$(top_srcdir)/valgrind_linux.supp $(top_srcdir)/kinetic_cpp_integration_test

endif

# Rule for generating docs
doc:
	echo "Generating API documentation with Doxygen"
	doxygen $(top_srcdir)/Doxyfile

# Rule for linting
lint:
	echo "Running style checker"
	$(top_srcdir)/bin/lint.sh $(top_srcdir)

distclean-local:
	rm -rf autom4te.cache m4
	rm -rf aclocal.m4 ar-lib compile config.guess config.h config.h.in config.h.in~ config.status
	rm -rf config.sub configure depcomp install-sh libtool ltmain.sh Makefile.in missing stamp-h1 test-driver
