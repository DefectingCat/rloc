CC = cc
CFLAGS = -Wall -Wextra -std=c11
INCLUDES = -I. -Itests

# Source files
SOURCES = main.c cli.c counter.c output.c util.c filelist.c language.c lang_defs.c strlit.c vcs.c diff.c
OBJECTS = $(SOURCES:.c=.o)

# Main target
rloc: $(SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(SOURCES)

# Test target
test_runner: tests/test_counter.c tests/test_framework.c counter.c strlit.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_counter.c tests/test_framework.c counter.c strlit.c

test_filelist_runner: tests/test_filelist.c tests/test_framework.c filelist.c util.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_filelist.c tests/test_framework.c filelist.c util.c

test_language_runner: tests/test_language.c tests/test_framework.c language.c lang_defs.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_language.c tests/test_framework.c language.c lang_defs.c

test_strlit_runner: tests/test_strlit.c tests/test_framework.c strlit.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_strlit.c tests/test_framework.c strlit.c

test_block_comments_runner: tests/test_block_comments.c tests/test_framework.c counter.c strlit.c language.c lang_defs.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_block_comments.c tests/test_framework.c counter.c strlit.c language.c lang_defs.c

test_continuation_runner: tests/test_continuation.c tests/test_framework.c counter.c strlit.c language.c lang_defs.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_continuation.c tests/test_framework.c counter.c strlit.c language.c lang_defs.c

test: test_runner test_filelist_runner test_language_runner test_strlit_runner test_block_comments_runner test_continuation_runner
	./test_runner
	./test_filelist_runner
	./test_language_runner
	./test_strlit_runner
	./test_block_comments_runner
	./test_continuation_runner

# Clean target
clean:
	rm -f rloc test_runner test_filelist_runner test_language_runner test_strlit_runner test_block_comments_runner test_continuation_runner

# Format target
fmt:
	clang-format -i *.c *.h tests/*.c tests/*.h

.PHONY: clean fmt test