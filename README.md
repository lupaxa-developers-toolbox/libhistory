<p align="center">
  <a href="https://github.com/lupaxa-developers-toolbox">
    <img src="https://raw.githubusercontent.com/the-lupaxa-project/brand-assets/master/logos/organisations/developers-toolbox/readme-logo.png" alt="Developers Toolbox" />
  </a>
</p>

<h1 align="center">Libhistory</h1>

C library for a fixed list of text lines.

Each history keeps a fixed number of lines. Index 0 is the newest line.
When the history is full, the next add drops the oldest line. The type is
opaque. The public header is `include/history.h`. It uses C linkage, so a
C++ translation unit can include it.

## Requirements

- GNU make.
- gcc, unless `CC` is set. The C skill uses gcc for syntax checks.
- cppcheck, for `make c-check`.

## Build

`make lib` writes the static archive `build/libhistory.a`. The archive is
built as C11 with `-Wall -Wextra -Wpedantic -Werror`. The compiler is gcc
unless `CC` is set. `make clean` removes `build/`.

```bash
make lib
cc -std=c11 -Iinclude -o demo demo.c build/libhistory.a
```

`make lib CC=clang` selects the compiler. `make lib CFLAGS=-g` adds flags to
that C11 build. `make` with no target prints the makefile help.

`make update` refreshes the makefile skills. `make lib` and `make test` do
not need that checkout. `make c-check` and `make version` do.

## Quick Start

Create a history of four lines, store one line, and destroy the history.
Destroy frees the copy.

```c
#include "history.h"

int main(void)
{
    history *record = NULL;
    const char *line = NULL;

    if (history_create(&record, 4) != HISTORY_OK) {
        return 1;
    }

    if (history_add(record, "started") != HISTORY_OK) {
        history_destroy(&record);
        return 1;
    }

    if (history_get(record, 0, &line) != HISTORY_OK) {
        history_destroy(&record);
        return 1;
    }

    history_destroy(&record);
    return 0;
}
```

## Lines

`history_add` copies the text it is given. An empty line is stored. A NULL
line is `HISTORY_INVAL`. A percent sign stays a percent sign.

`history_add_formatted` formats a line with `printf` rules and stores the
result. `history_add_formatted(record, "%d", 3)` stores `"3"`. A NULL format
is `HISTORY_INVAL`.

Index 0 is the newest line. `history_get` returns a pointer owned by the
history. It stays valid until that line is dropped, `history_clear`, or
`history_destroy`. The caller does not free it.

A full history drops the oldest line only after the new line exists. A
failed add leaves the stored lines unchanged.

## Ownership

The history owns every stored line. `history_clear` frees the lines and
leaves the history empty, with the same capacity. `history_destroy` frees
the lines and the history, then sets `*history` to NULL.

> **Note:** Callers serialise use of a given history.

## Return Codes

Every function that can fail returns an `int`.

| Code             | Value | Meaning                                                                                                       |
| ---------------- | ----- | ------------------------------------------------------------------------------------------------------------- |
| `HISTORY_OK`     | `0`   | The call succeeded.                                                                                           |
| `HISTORY_NOMEM`  | `-1`  | Allocation failed. The history is unchanged. Create sets `*history` to NULL.                                  |
| `HISTORY_INVAL`  | `-2`  | The history, line, or format is missing, the capacity is 0, or the index is past the last line.               |

`history_destroy(NULL)` returns `HISTORY_INVAL`. `history_destroy` on a
pointer that is already NULL returns `HISTORY_OK` and leaves it NULL.

A capacity of 0 leaves `*history` unchanged. A capacity that cannot be
allocated sets `*history` to NULL.

## Functions

```c
typedef struct history history;

int history_create(history **history, size_t capacity);
int history_destroy(history **history);
int history_clear(history *history);
int history_add(history *history, const char *line);
int history_add_formatted(history *history, const char *fmt, ...);
int history_get(const history *history, size_t index, const char **line);
size_t history_count(const history *history);
size_t history_capacity(const history *history);
```

| Name                     | Purpose                                                 |
| ------------------------ | ------------------------------------------------------- |
| `history_create`         | Allocate a history that keeps at most `capacity` lines. |
| `history_destroy`        | Free the lines and the history, and clear `*history`.   |
| `history_clear`          | Free the lines and leave the history empty.             |
| `history_add`            | Copy a line and store it as the newest line.            |
| `history_add_formatted`  | Format a line and store that text as the newest line.   |
| `history_get`            | Read the line at `index`. Index 0 is the newest.        |
| `history_count`          | Return how many lines are stored.                       |
| `history_capacity`       | Return the capacity chosen at creation.                 |

`history_count(NULL)` and `history_capacity(NULL)` return 0. A missing index
leaves the out-pointer unchanged.

## Empty History

A new history has a count of 0. Destroying it returns `HISTORY_OK` and sets
the caller's pointer to NULL. Destroying that NULL pointer again returns
`HISTORY_OK`.

## Version

The version text is the macro `HISTORY_VERSION_STRING` in
`include/history.h`. It is a dotted semantic version. `.bumpversion.toml`
is the version source, and a bump updates that macro.

After `make init`, `make version` prints the version from
`.bumpversion.toml`.

## Development

```bash
make update
make lib
make check
make clean
```

`make c-check` syntax-checks every `.c` and `.h` file, then runs cppcheck.
`make check` runs `c-check` and then the behaviour tests. `make test` runs
only the behaviour tests.

<a href="https://github.com/the-lupaxa-project">
    <img src="https://raw.githubusercontent.com/the-lupaxa-project/brand-assets/master/logos/components/footer-for-child-orgs.svg" alt="The Lupaxa Project Footer" width="100%" />
</a>
