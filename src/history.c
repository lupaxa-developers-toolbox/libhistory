/* Fixed list of text lines, newest at index 0.
   The public contract is in include/history.h. Lines are copied. A full
   history drops the oldest line only after the new line has been built. */

/* Quoted path so a syntax check without -I still finds the public header. */
#include "../include/history.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* lines is a ring of capacity slots. newest is the physical index of the
   most recent line and is meaningful when count is not 0. */
struct history {
    char **lines;
    size_t capacity;
    size_t count;
    size_t newest;
};

static char *dup_text(const char *text)
{
    size_t length = strlen(text);
    char *copy;

    if (length == SIZE_MAX) {
        return NULL;
    }
    copy = malloc(length + 1);
    if (copy == NULL) {
        return NULL;
    }
    memcpy(copy, text, length + 1);
    return copy;
}

static size_t physical(const history *history, size_t index)
{
    return (history->newest + history->capacity - index) % history->capacity;
}

static int push_line(history *history, char *line)
{
    size_t slot;

    if (history->count < history->capacity) {
        slot = (history->count == 0) ? 0 : (history->newest + 1) % history->capacity;
        history->lines[slot] = line;
        history->newest = slot;
        history->count++;
        return HISTORY_OK;
    }

    slot = (history->newest + 1) % history->capacity;
    free(history->lines[slot]);
    history->lines[slot] = line;
    history->newest = slot;
    return HISTORY_OK;
}

static int format_line(const char *fmt, va_list args, char **out)
{
    va_list measure;
    int length;
    char *text;

    va_copy(measure, args);
    length = vsnprintf(NULL, 0, fmt, measure);
    va_end(measure);
    if (length < 0) {
        return HISTORY_INVAL;
    }
    if ((size_t)length == SIZE_MAX) {
        return HISTORY_NOMEM;
    }
    text = malloc((size_t)length + 1);
    if (text == NULL) {
        return HISTORY_NOMEM;
    }
    if (vsnprintf(text, (size_t)length + 1, fmt, args) < 0) {
        free(text);
        return HISTORY_INVAL;
    }
    *out = text;
    return HISTORY_OK;
}

int history_create(history **out, size_t capacity)
{
    history *created;

    if (out == NULL || capacity == 0) {
        return HISTORY_INVAL;
    }
    if (capacity > SIZE_MAX / sizeof(char *)) {
        *out = NULL;
        return HISTORY_NOMEM;
    }

    created = calloc(1, sizeof *created);
    if (created == NULL) {
        *out = NULL;
        return HISTORY_NOMEM;
    }
    created->lines = calloc(capacity, sizeof *created->lines);
    if (created->lines == NULL) {
        free(created);
        *out = NULL;
        return HISTORY_NOMEM;
    }
    created->capacity = capacity;
    *out = created;
    return HISTORY_OK;
}

int history_clear(history *history)
{
    size_t index;

    if (history == NULL) {
        return HISTORY_INVAL;
    }
    for (index = 0; index < history->count; index++) {
        size_t slot = physical(history, index);

        free(history->lines[slot]);
        history->lines[slot] = NULL;
    }
    history->count = 0;
    history->newest = 0;
    return HISTORY_OK;
}

int history_destroy(history **history)
{
    if (history == NULL) {
        return HISTORY_INVAL;
    }
    if (*history == NULL) {
        return HISTORY_OK;
    }
    history_clear(*history);
    free((*history)->lines);
    free(*history);
    *history = NULL;
    return HISTORY_OK;
}

int history_add(history *history, const char *line)
{
    char *copy;

    if (history == NULL || line == NULL) {
        return HISTORY_INVAL;
    }
    copy = dup_text(line);
    if (copy == NULL) {
        return HISTORY_NOMEM;
    }
    return push_line(history, copy);
}

int history_add_formatted(history *history, const char *fmt, ...)
{
    va_list args;
    char *text = NULL;
    int result;

    if (history == NULL || fmt == NULL) {
        return HISTORY_INVAL;
    }
    va_start(args, fmt);
    result = format_line(fmt, args, &text);
    va_end(args);
    if (result != HISTORY_OK) {
        return result;
    }
    return push_line(history, text);
}

int history_get(const history *history, size_t index, const char **line)
{
    if (history == NULL || line == NULL) {
        return HISTORY_INVAL;
    }
    if (index >= history->count) {
        return HISTORY_INVAL;
    }
    *line = history->lines[physical(history, index)];
    return HISTORY_OK;
}

size_t history_count(const history *history)
{
    if (history == NULL) {
        return 0;
    }
    return history->count;
}

size_t history_capacity(const history *history)
{
    if (history == NULL) {
        return 0;
    }
    return history->capacity;
}
