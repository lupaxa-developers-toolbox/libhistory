/* Quoted path so a syntax check without -I still finds the public header. */
#include "../include/history.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int failures = 0;
static int tests_run = 0;
static int tests_failed = 0;
static const char *current_test = NULL;

static void expect(int condition, const char *file, int line, const char *expr)
{
    if (!condition) {
        printf("FAIL %s %s:%d: %s\n",
               current_test != NULL ? current_test : "?",
               file, line, expr);
        fflush(stdout);
        failures++;
    }
}

#define EXPECT(cond) expect((cond), __FILE__, __LINE__, #cond)

static int semver_major_minor(const char *version)
{
    int parts = 0;

    if (version == NULL) {
        return 0;
    }

    while (parts < 2) {
        if (*version < '0' || *version > '9') {
            return 0;
        }
        while (*version >= '0' && *version <= '9') {
            version++;
        }
        parts++;
        if (parts == 2) {
            return 1;
        }
        if (*version != '.') {
            return 0;
        }
        version++;
    }
    return 0;
}

static void test_version_string_is_semver(void)
{
    EXPECT(semver_major_minor(HISTORY_VERSION_STRING));
}

static void test_create_and_destroy(void)
{
    history *record = NULL;
    history *kept = NULL;

    EXPECT(history_create(NULL, 4) == HISTORY_INVAL);
    EXPECT(history_create(&record, 0) == HISTORY_INVAL);
    EXPECT(record == NULL);
    EXPECT(history_create(&kept, 4) == HISTORY_OK);
    EXPECT(history_capacity(kept) == 4);
    EXPECT(history_count(kept) == 0);
    EXPECT(history_count(NULL) == 0);
    EXPECT(history_capacity(NULL) == 0);
    record = kept;
    EXPECT(history_create(&record, 0) == HISTORY_INVAL);
    EXPECT(record == kept);
    EXPECT(history_clear(NULL) == HISTORY_INVAL);
    EXPECT(history_clear(kept) == HISTORY_OK);
    EXPECT(history_count(kept) == 0);
    EXPECT(history_destroy(&record) == HISTORY_OK);
    EXPECT(record == NULL);
    EXPECT(history_destroy(&record) == HISTORY_OK);
    EXPECT(history_destroy(NULL) == HISTORY_INVAL);

    record = kept;
    EXPECT(history_create(&record, SIZE_MAX) == HISTORY_NOMEM);
    EXPECT(record == NULL);
}

static void test_add_copies_the_line(void)
{
    history *record = NULL;
    const char *line = "keep";
    char text[] = "hello";

    EXPECT(history_create(&record, 4) == HISTORY_OK);
    EXPECT(history_add(NULL, "a") == HISTORY_INVAL);
    EXPECT(history_add(record, NULL) == HISTORY_INVAL);
    EXPECT(history_count(record) == 0);
    EXPECT(history_add(record, "") == HISTORY_OK);
    EXPECT(history_add(record, text) == HISTORY_OK);
    EXPECT(history_add(record, "%d") == HISTORY_OK);
    text[0] = 'H';
    EXPECT(history_count(record) == 3);
    EXPECT(history_get(record, 0, &line) == HISTORY_OK);
    EXPECT(strcmp(line, "%d") == 0);
    EXPECT(history_get(record, 1, &line) == HISTORY_OK);
    EXPECT(strcmp(line, "hello") == 0);
    EXPECT(history_get(record, 2, &line) == HISTORY_OK);
    EXPECT(strcmp(line, "") == 0);
    line = "keep";
    EXPECT(history_get(record, 3, &line) == HISTORY_INVAL);
    EXPECT(strcmp(line, "keep") == 0);
    EXPECT(history_get(record, 0, NULL) == HISTORY_INVAL);
    EXPECT(history_get(NULL, 0, &line) == HISTORY_INVAL);
    EXPECT(history_destroy(&record) == HISTORY_OK);
}

static void test_add_formatted_uses_printf_rules(void)
{
    history *record = NULL;
    const char *line = "keep";

    EXPECT(history_create(&record, 4) == HISTORY_OK);
    EXPECT(history_add(record, "stay") == HISTORY_OK);
    EXPECT(history_add_formatted(NULL, "%d", 1) == HISTORY_INVAL);
    EXPECT(history_add_formatted(record, NULL) == HISTORY_INVAL);
    EXPECT(history_get(record, 0, &line) == HISTORY_OK);
    EXPECT(strcmp(line, "stay") == 0);
    EXPECT(history_add_formatted(record, "%d", 3) == HISTORY_OK);
    EXPECT(history_add_formatted(record, "100%%") == HISTORY_OK);
    EXPECT(history_add_formatted(record, "") == HISTORY_OK);
    EXPECT(history_get(record, 0, &line) == HISTORY_OK);
    EXPECT(strcmp(line, "") == 0);
    EXPECT(history_get(record, 1, &line) == HISTORY_OK);
    EXPECT(strcmp(line, "100%") == 0);
    EXPECT(history_get(record, 2, &line) == HISTORY_OK);
    EXPECT(strcmp(line, "3") == 0);
    EXPECT(history_get(record, 3, &line) == HISTORY_OK);
    EXPECT(strcmp(line, "stay") == 0);
    EXPECT(history_destroy(&record) == HISTORY_OK);
}

static void test_full_history_drops_the_oldest(void)
{
    history *record = NULL;
    const char *line = "keep";

    EXPECT(history_create(&record, 1) == HISTORY_OK);
    EXPECT(history_add(record, "only") == HISTORY_OK);
    EXPECT(history_add(record, "next") == HISTORY_OK);
    EXPECT(history_count(record) == 1);
    EXPECT(history_get(record, 0, &line) == HISTORY_OK);
    EXPECT(strcmp(line, "next") == 0);
    EXPECT(history_get(record, 1, &line) == HISTORY_INVAL);
    EXPECT(history_destroy(&record) == HISTORY_OK);

    EXPECT(history_create(&record, 2) == HISTORY_OK);
    EXPECT(history_add(record, "a") == HISTORY_OK);
    EXPECT(history_add(record, "b") == HISTORY_OK);
    EXPECT(history_add(record, "c") == HISTORY_OK);
    EXPECT(history_count(record) == 2);
    EXPECT(history_get(record, 0, &line) == HISTORY_OK);
    EXPECT(strcmp(line, "c") == 0);
    EXPECT(history_get(record, 1, &line) == HISTORY_OK);
    EXPECT(strcmp(line, "b") == 0);
    line = "keep";
    EXPECT(history_get(record, 2, &line) == HISTORY_INVAL);
    EXPECT(strcmp(line, "keep") == 0);
    EXPECT(history_add_formatted(record, "%s", "d") == HISTORY_OK);
    EXPECT(history_get(record, 0, &line) == HISTORY_OK);
    EXPECT(strcmp(line, "d") == 0);
    EXPECT(history_get(record, 1, &line) == HISTORY_OK);
    EXPECT(strcmp(line, "c") == 0);
    EXPECT(history_destroy(&record) == HISTORY_OK);
}

static void test_clear_and_separate_histories(void)
{
    history *first = NULL;
    history *second = NULL;
    const char *line = "keep";

    EXPECT(history_create(&first, 3) == HISTORY_OK);
    EXPECT(history_create(&second, 3) == HISTORY_OK);
    EXPECT(history_add(first, "a") == HISTORY_OK);
    EXPECT(history_add(first, "b") == HISTORY_OK);
    EXPECT(history_add(second, "other") == HISTORY_OK);
    EXPECT(history_clear(first) == HISTORY_OK);
    EXPECT(history_count(first) == 0);
    EXPECT(history_capacity(first) == 3);
    line = "keep";
    EXPECT(history_get(first, 0, &line) == HISTORY_INVAL);
    EXPECT(strcmp(line, "keep") == 0);
    EXPECT(history_get(second, 0, &line) == HISTORY_OK);
    EXPECT(strcmp(line, "other") == 0);
    EXPECT(history_add(first, "fresh") == HISTORY_OK);
    EXPECT(history_get(first, 0, &line) == HISTORY_OK);
    EXPECT(strcmp(line, "fresh") == 0);
    EXPECT(history_count(second) == 1);
    EXPECT(history_destroy(&first) == HISTORY_OK);
    EXPECT(history_destroy(&second) == HISTORY_OK);
}

static void run_test(const char *name, void (*fn)(void))
{
    int before = failures;

    current_test = name;
    fn();
    tests_run++;
    if (failures == before) {
        printf("tests/test_history.c::%s PASSED\n", name);
    } else {
        tests_failed++;
        printf("tests/test_history.c::%s FAILED\n", name);
    }
    fflush(stdout);
}

int main(void)
{
    int passed;

    run_test("test_version_string_is_semver", test_version_string_is_semver);
    run_test("test_create_and_destroy", test_create_and_destroy);
    run_test("test_add_copies_the_line", test_add_copies_the_line);
    run_test("test_add_formatted_uses_printf_rules",
             test_add_formatted_uses_printf_rules);
    run_test("test_full_history_drops_the_oldest", test_full_history_drops_the_oldest);
    run_test("test_clear_and_separate_histories", test_clear_and_separate_histories);

    passed = tests_run - tests_failed;
    if (tests_failed != 0) {
        printf("%d failed, %d passed, %d assertion(s) failed\n",
               tests_failed, passed, failures);
        return 1;
    }
    printf("%d passed\n", passed);
    return 0;
}
