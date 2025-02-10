#include "unity.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../../../examples/systemcalls/systemcalls.h"
#include "file_write_commands.h"



void test_systemcalls_modified()
{
    printf("Running tests at %s : function %s\n",__FILE__,__func__);
    
    // 1) Perform system() operation using do_system()
    TEST_ASSERT_TRUE_MESSAGE(do_system("echo this is a test > " REDIRECT_FILE),
                             "do_system call should return true running echo command");
    
    // 2) Obtain the returned value from return_string_validation() from file_write_commands.h
    char *test_string = return_string_validation(REDIRECT_FILE);
    printf("Value returned from return_string_validation: %s\n", test_string);

    // 3) Unity assertion comparing the strings
    TEST_ASSERT_EQUAL_STRING_MESSAGE("this is a test", test_string,
                                     "Did not find \"this is a test\" in output of echo command. Is your system() function implemented properly?");
    free((void *)test_string);

    // 4) Test with strncmp for $HOME full path
    TEST_ASSERT_TRUE_MESSAGE(do_system("echo \"home is $HOME\" > " REDIRECT_FILE),
                             "do_system call should return true running echo command");
    test_string = return_string_validation(REDIRECT_FILE);
    int test_value = strncmp(test_string, "home is /", 9);
    printf("system() echo home is $HOME returned: %s\n", test_string);
    
    // 5) Test if $HOME is expanded
    TEST_ASSERT_EQUAL_INT16_MESSAGE(test_value, 0,
                                    "The first 9 chars echoed should be \"home is /\". The last chars will include the content of the $HOME variable");
    
    test_value = strncmp(test_string, "home is $HOME", 9);
    TEST_ASSERT_NOT_EQUAL_INT16_MESSAGE(test_value, 0,
                                        "The $HOME parameter should be expanded when using system()");
    free((void *)test_string);

    // 6) Perform fork(), execv() and wait() manually using do_exec()
    TEST_ASSERT_FALSE_MESSAGE(do_exec(2, "echo", "Testing execv implementation with echo"),
                              "The exec() function should have returned false since echo was not specified with absolute path as a command and PATH expansion is not performed.");
    
    TEST_ASSERT_FALSE_MESSAGE(do_exec(3, "/usr/bin/test", "-f", "echo"),
                              "The exec() function should have returned false since echo was not specified with absolute path in argument to the test executable.");
    
    TEST_ASSERT_TRUE_MESSAGE(do_exec(3, "/usr/bin/test", "-f", "/bin/echo"),
                             "The function should return true since /bin/echo represents the echo command and test -f verifies this is a valid file");

    // 7) Perform redirection with do_exec_redirect()
    do_exec_redirect(REDIRECT_FILE, 3, "/bin/sh", "-c", "echo home is $HOME");
    test_string = return_string_validation(REDIRECT_FILE);
    TEST_ASSERT_NOT_NULL_MESSAGE(test_string, "Nothing written to file at " REDIRECT_FILE);
    
    if (test_string != NULL) {
        int test_value = strncmp(test_string, "home is /", 9);
        printf("execv /bin/sh -c echo home is $HOME returned %s\n", test_string);
        
        // 8) Unity assertion for redirection output
        TEST_ASSERT_EQUAL_INT16_MESSAGE(test_value, 0,
                                        "The first 9 chars echoed should be \"home is /\". The last chars will include the content of the $HOME variable.");
        
        test_value = strncmp(test_string, "home is $HOME", 9);
        TEST_ASSERT_NOT_EQUAL_INT16_MESSAGE(test_value, 0,
                                            "The $HOME parameter should be expanded when using /bin/sh with do_exec()");
        free(test_string);
    }

    // 9) Test another case for redirection with $HOME variable
    do_exec_redirect(REDIRECT_FILE, 2, "/bin/echo", "home is $HOME");
    test_string = return_string_validation(REDIRECT_FILE);
    TEST_ASSERT_NOT_NULL_MESSAGE(test_string, "Nothing written to file at " REDIRECT_FILE);
    
    if (test_string != NULL) {
        printf("execv /bin/echo home is $HOME returned %s\n", test_string);
        
        // 10) Unity assertion for the correct result from execv with echo command
        TEST_ASSERT_EQUAL_STRING_MESSAGE("home is $HOME", test_string,
                                         "The variable $HOME should not be expanded using execv()");
        free(test_string);
    }
}

