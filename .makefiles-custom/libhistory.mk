# Static library build. The c skill syntax-checks and runs cppcheck.
# It does not compile or link.

override CFLAGS += -std=c11 -Wall -Wextra -Wpedantic -Werror -Iinclude
override CPPCHECK_FLAGS += -Iinclude --std=c11

HISTORY_BUILD := build
HISTORY_LIB := $(HISTORY_BUILD)/libhistory.a
HISTORY_OBJ := $(HISTORY_BUILD)/history.o
HISTORY_TEST := $(HISTORY_BUILD)/test_history

.PHONY: lib test check clean

lib: $(HISTORY_LIB)

$(HISTORY_OBJ): src/history.c include/history.h
	mkdir -p $(HISTORY_BUILD)
	$(CC) $(CFLAGS) -c -o $@ src/history.c

$(HISTORY_LIB): $(HISTORY_OBJ)
	$(AR) rcs $@ $(HISTORY_OBJ)

$(HISTORY_TEST): tests/test_history.c include/history.h $(HISTORY_LIB)
	$(CC) $(CFLAGS) -o $@ tests/test_history.c $(HISTORY_LIB) $(LDFLAGS)

test: $(HISTORY_TEST)
	./$(HISTORY_TEST)

check: $(if $(filter c,$(SKILLS)),c-check) test

clean:
	rm -rf $(HISTORY_BUILD)
