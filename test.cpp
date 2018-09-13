#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>


/* Test runner may be provided options, such
   as to enable colored output, to run only a
   specific test or a group of tests, etc. This
   will return the number of failed tests. */

int main(int argc, char ** argv)
{
    RUN_ALL_TESTS(argc, argv);
}