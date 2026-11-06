#ifndef HISTORY_H
#define HISTORY_H

#include <stddef.h>

#define HISTORY_VERSION_STRING "0.1.0"

/* HISTORY_OK is success. HISTORY_NOMEM means allocation failed and the
   history was left unchanged, except history_create which sets *history
   to NULL. HISTORY_INVAL means a missing history, a missing line or
   format, a capacity of 0, or an index past the last stored line. */
#define HISTORY_OK 0
#define HISTORY_NOMEM (-1)
#define HISTORY_INVAL (-2)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct history history;

/* Allocate a history that keeps at most capacity lines. A capacity of 0
   leaves *history unchanged. On allocation failure *history is NULL.
   Index 0 is the newest line. */
int history_create(history **history, size_t capacity);

/* Free every stored line and the history. *history is set to NULL.
   Destroying an already NULL history is success. */
int history_destroy(history **history);

/* Free every stored line and leave the history empty. */
int history_clear(history *history);

/* Copy line and store it as the newest line. An empty line is stored.
   A NULL line is HISTORY_INVAL. When the history is full, the oldest line
   is dropped only after the new line exists. On failure the history is
   unchanged. The text is copied as given, so a percent sign stays a
   percent sign. */
int history_add(history *history, const char *line);

/* Format fmt with printf rules and store that text as the newest line.
   A NULL format is HISTORY_INVAL. A formatting error leaves the history
   unchanged. This is the formatted form of history_add. */
int history_add_formatted(history *history, const char *fmt, ...);

/* Write the line at index to *line. Index 0 is the newest line. The
   pointer is owned by the history and stays valid until that line is
   dropped, history_clear, or history_destroy. On failure *line is
   unchanged. */
int history_get(const history *history, size_t index, const char **line);

/* Return the number of stored lines. A NULL history returns 0. */
size_t history_count(const history *history);

/* Return the capacity chosen at creation. A NULL history returns 0. */
size_t history_capacity(const history *history);

#ifdef __cplusplus
}
#endif

#endif /* HISTORY_H */
