Test Support
============

This directory contains source code for testing for the OSAL library.
There are currently 2 types of tests that are performed:
  - unit tests
  - integration tests


Unit Tests
----------
Located in the "unit" sub-directory, the goal of the unit test is to verify that
given a specific input the expected output is achieved.   Unit tests should be
fast and cover all code paths (i.e. complete code coverage).  It is intended to
have tests that only test a single function (i.e. unit) any calls outside that
functions should generally be "mocked" out.  This way even if the code that a
unit depends on is mis-behaving, the unit test should pass, because the failure
is not in the "function" itself.

Testing for unit tests should include covering all code paths.  Testing the
input and output parameters of the function.  For example, if a function takes
a `const char *` as a parameter, what happens if a `NULL` is passed, as well
as a valid pointer value.  Even, though the first test may not increase any
coverage results, it's still an expected test.

Speed is a concern with unit tests, they are expected to be run repeatedly,
and should be complete very fast.  In the order of "milliseconds" on a robust
build system.  For example testing a `sleep` function with a parameter of 10
seconds.  May not sleep for 10 seconds, it may not sleep at all.  The point is
to test that the function return success, if given a valid parameter and that
would be successfully passed to the underlying operating system call.


Integration Tests
-----------------
Integration tests are tests that uses the functions and should report a
consistent output.  This calls, do not utilize 'mocking' because their purpose
is to test, how a user would call the functions.  Are they producing the
expected results.  These tests are good for ensuring that the library runs
consistently on all the supported operating system.

There is not requirement for these tests to be fast, in fact some tests are
guaranteed to be slow (on a succesful operation) for example, `sleep`.  If
succesful, a test calling `sleep` for 10 seconds, would actually have to wait
10 seconds to determine if the sleep works.


System Tests
------------
Systems tests are intended as performance tests, to see how the various
functions behave in various conditions.  These tests are not included in this
repository, and may have different results on different operating systems.
This allows for predicting how the same code using this library may behave
differently on different systems.

