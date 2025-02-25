#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include "harness/unity.h"
#include "../src/lab.h"  // Adjust the path as needed

// Set up function to run before each test
void setUp(void) {
    /* Bypass terminal control and built-in exit during tests */
    setenv("SKIP_TC", "1", 1);
    setenv("SKIP_EXIT", "1", 1);
}

// Tear down function to run after each test
void tearDown(void) {
    unsetenv("SKIP_TC");
    unsetenv("SKIP_EXIT");
}

// Test parsing a simple command with an argument
void test_cmd_parse2(void)
{
    char *stng = (char*)malloc(sizeof(char) * 7);
    strcpy(stng, "foo -v");
    char **actual = cmd_parse(stng);

    // Verify that the parsed tokens match the expected values
    TEST_ASSERT_EQUAL_STRING("foo", actual[0]);
    TEST_ASSERT_EQUAL_STRING("-v", actual[1]);
    TEST_ASSERT_NULL(actual[2]);  // Ensure the array is properly terminated

    cmd_free(actual);
    free(stng);
}

// Test parsing a command with multiple arguments
void test_cmd_parse(void)
{
    char **rval = cmd_parse("ls -a -l");
    TEST_ASSERT_NOT_NULL(rval);
    TEST_ASSERT_EQUAL_STRING("ls", rval[0]);
    TEST_ASSERT_EQUAL_STRING("-a", rval[1]);
    TEST_ASSERT_EQUAL_STRING("-l", rval[2]);
    TEST_ASSERT_NULL(rval[3]);  // Ensure null termination
    cmd_free(rval);
}

// Test trimming a string with no whitespace
void test_trim_white_no_whitespace(void)
{
    char *line = strdup("ls -a");
    char *rval = trim_white(line);
    TEST_ASSERT_EQUAL_STRING("ls -a", rval);
    free(line);
}

// Test trimming a string with leading whitespace
void test_trim_white_start_whitespace(void)
{
    char *line = (char*) calloc(10, sizeof(char));
    strncpy(line, " ls -a", 10);
    char *rval = trim_white(line);
    TEST_ASSERT_EQUAL_STRING("ls -a", rval);
    free(line);
}

// Test trimming a string with trailing whitespace
void test_trim_white_end_whitespace(void)
{
    char *line = (char*) calloc(10, sizeof(char));
    strncpy(line, "ls -a ", 10);
    char *rval = trim_white(line);
    TEST_ASSERT_EQUAL_STRING("ls -a", rval);
    free(line);
}

// Test trimming a string with both leading and trailing whitespace
void test_trim_white_both_whitespace_single(void)
{
    char *line = (char*) calloc(10, sizeof(char));
    strncpy(line, " ls -a ", 10);
    char *rval = trim_white(line);
    TEST_ASSERT_EQUAL_STRING("ls -a", rval);
    free(line);
}

// Duplicate of the previous test
void test_trim_white_both_whitespace_double(void)
{
    char *line = (char*) calloc(10, sizeof(char));
    strncpy(line, " ls -a ", 10);
    char *rval = trim_white(line);
    TEST_ASSERT_EQUAL_STRING("ls -a", rval);
    free(line);
}

// Test trimming a string that contains only whitespace
void test_trim_white_all_whitespace(void)
{
    char *line = (char*) calloc(10, sizeof(char));
    strncpy(line, " ", 10);
    char *rval = trim_white(line);
    TEST_ASSERT_EQUAL_STRING("", rval);  // Should return an empty string
    free(line);
}

// Test trimming a string with mostly whitespace but some content
void test_trim_white_mostly_whitespace(void)
{
    char *line = (char*) calloc(10, sizeof(char));
    strncpy(line, " a ", 10);
    char *rval = trim_white(line);
    TEST_ASSERT_EQUAL_STRING("a", rval);
    free(line);
}

// Test default shell prompt when MY_PROMPT is not set
void test_get_prompt_default(void)
{
    unsetenv("MY_PROMPT");
    char *prompt = get_prompt("MY_PROMPT");
    TEST_ASSERT_EQUAL_STRING("shell>", prompt);
    free(prompt);
}

// Test shell prompt when MY_PROMPT is set to a custom value
void test_get_prompt_custom(void)
{
    setenv("MY_PROMPT", "foo>", 1);
    char *prompt = get_prompt("MY_PROMPT");
    TEST_ASSERT_EQUAL_STRING("foo>", prompt);
    free(prompt);
    unsetenv("MY_PROMPT");
}

// Test changing directory to home
void test_ch_dir_home(void)
{
    char *orig = getcwd(NULL, 0);
    char **cmd = cmd_parse("cd");
    char *expected = getenv("HOME");
    change_dir(cmd);
    char *actual = getcwd(NULL, 0);
    TEST_ASSERT_EQUAL_STRING(expected, actual);
    free(actual);
    free(orig);
    cmd_free(cmd);
}

// Test changing directory to root (/)
void test_ch_dir_root(void)
{
    char *orig = getcwd(NULL, 0);
    char **cmd = cmd_parse("cd /");
    change_dir(cmd);
    char *actual = getcwd(NULL, 0);
    TEST_ASSERT_EQUAL_STRING("/", actual);
    free(actual);
    chdir(orig);  // Restore the original directory
    free(orig);
    cmd_free(cmd);
}

// Test handling of "exit" as a built-in command
void test_do_builtin_exit(void) {
    struct shell sh;
    sh_init(&sh);
    char *cmd[] = {"exit", NULL};

    // Ensure the function recognizes "exit" as a valid built-in command
    TEST_ASSERT_TRUE(do_builtin(&sh, cmd));

    sh_destroy(&sh);
}

// Test handling of "cd" with an invalid path
void test_do_builtin_cd_invalid(void) {
    struct shell sh;
    sh_init(&sh);
    char *cmd[] = {"cd", "/invalid/path", NULL};

    // Ensure the function still returns true even though the command fails
    TEST_ASSERT_TRUE(do_builtin(&sh, cmd));

    sh_destroy(&sh);
}

// Test handling of "cd" without arguments (should go to HOME)
void test_do_builtin_cd_home(void) {
    struct shell sh;
    sh_init(&sh);
    char *cmd[] = {"cd", NULL};

    TEST_ASSERT_TRUE(do_builtin(&sh, cmd));

    char *expected = getenv("HOME");
    char *actual = getcwd(NULL, 0);
    TEST_ASSERT_EQUAL_STRING(expected, actual);
    free(actual);

    sh_destroy(&sh);
}

// Test handling of "history" command (even if unimplemented)
void test_do_builtin_history(void) {
    struct shell sh;
    sh_init(&sh);
    char *cmd[] = {"history", NULL};

    TEST_ASSERT_TRUE(do_builtin(&sh, cmd));

    sh_destroy(&sh);
}

// Test initializing the shell struct
void test_sh_init(void) {
    struct shell sh;
    sh_init(&sh);

    // Ensure the shell prompt is set
    TEST_ASSERT_NOT_NULL(sh.prompt);

    sh_destroy(&sh);
}

// Test destroying the shell struct
void test_sh_destroy(void) {
    struct shell sh;
    sh_init(&sh);
    sh_destroy(&sh);

    // Ensure the shell prompt is freed
    TEST_ASSERT_NULL(sh.prompt);
}

// Main function to run all tests
int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_cmd_parse);
    RUN_TEST(test_cmd_parse2);
    RUN_TEST(test_trim_white_no_whitespace);
    RUN_TEST(test_trim_white_start_whitespace);
    RUN_TEST(test_trim_white_end_whitespace);
    RUN_TEST(test_trim_white_both_whitespace_single);
    RUN_TEST(test_trim_white_both_whitespace_double);
    RUN_TEST(test_trim_white_all_whitespace);
    RUN_TEST(test_trim_white_mostly_whitespace);
    RUN_TEST(test_get_prompt_default);
    RUN_TEST(test_get_prompt_custom);
    RUN_TEST(test_ch_dir_home);
    RUN_TEST(test_ch_dir_root);
    RUN_TEST(test_do_builtin_exit);
    RUN_TEST(test_do_builtin_cd_invalid);
    RUN_TEST(test_do_builtin_cd_home);
    RUN_TEST(test_do_builtin_history);
    RUN_TEST(test_sh_init);
    RUN_TEST(test_sh_destroy);

    return UNITY_END();
}
