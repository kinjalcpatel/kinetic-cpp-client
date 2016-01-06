[![Build Status](https://travis-ci.org/Kinetic/kinetic-cpp-client.svg?branch=master)](https://travis-ci.org/Kinetic/kinetic-cpp-client)

Introduction
============
This repo contains code for producing C++ kinetic client library. The
C++ library currently does not support Windows at this time because of
existing library requirements.


Kinetic Protocol Version
========================
The kinetic-protocol header and library files are probled and used at
the build time. These external kinetic-protocol files dectate the
version of the supported kinetic protocol.


Build Dependencies
==================
* autoconf-2.69
* kientic-protocol
* openssl, openssl-devel
* protobuf, protobuf-devel
* gflags, gflags-devel (except v2.1.1, which is broken)
* glog, glog-devel
* gtest, gtest-devel
* gmock, gmock-devel
* Valgrind for memory tests
* Doxygen/graphviz for generating documentation

Note:
  Currently, packages for kinetic-protocol are not availables in
distributions repositories, so one has to download kinetic-proto from
https://github.com/Kinetic/kinetic-protocol and follow it's build
instructions to satisfy the kinetic-protocol dependency.

Also for developement on Ubuntu 14.04 LTS, it has only devel(headers)
packages of gmock and gtest, which doesnt provide shared library for
respective package. Also gmock devel package refers to macros which
are not present in gtest package.
So developers are adviced to download the latest v1.7.0 release from
github, build it and install it manually.

Initial Setup
=============
To build kinetic-cpp-client library and tests, make sure all the
dependencies mentioned above are installed. Then run the following
standard autotools build steps.

1. To create build makefiles using autotools, run autogen.sh script from
the top source code directory:

        ./autogen.sh

2. Run the configure script from the source directory to generate Makefile:

        ./configure

The configure script expects the dependencies header files to be included
from "${prefix}/include" directory. If the header files include path is
different, then use CPPFLAGS variable to provide the include path.
For example:

        export CPPFLAGS="-I<the include path>"
        ./configure

        or

       ./configure CPPFLAGS="-I<the include path>"

Similarly, if the library  path is different than "${prefix}/lib" then
use LDFLAGS variable to specify the library include path.
For example:

        export LDFLAGS="-L<the library path>"
        ./configure

        or

        ./configure LDFLAGS="-L<the library path>"

Note: User can disable test targets by disabling enable-test option. For example,

        ./configure --enable-test=no

3. To build the kientic-cpp-client library, run `make` from build directory.

4. To run the unit test suite, run `make check`. Tests results
will appear on stdout and a JUnit report will be written to `gtestresults.xml`

There is also an integration test suite. This suite reads the environment
variable `KINETIC_PATH` to determine a simulator executable to run tests
against. If that variable is not set, it instead assumes that a Kinetic server
is running on port 8123 on `localhost`. To run the integration tests, set
`KINETIC_PATH` if appropriate and run `make integration_test`. This will write
a JUnit report to `integrationresults.xml`.

To run tests with leak check, run `make test_valgrind` for the unit test
suite or `make integration_test_valgrind` for the integration test suite.

To check code style, run `make lint`. Violations will be printed on stdout.

To generate documentation, run `make doc`. HTML documentation will be generated in `docs/`

To apply licenses, run something like `./apply_license.sh my_new_file.cc` or `./apply_license.sh src/*.h`

5. Install the kinetic-cpp-client library,

        make install

6. Uninstall the kinetic-cpp-client library,

        make uninstall

7. Clean the directory for another clean build.

        make clean
        make distclean
