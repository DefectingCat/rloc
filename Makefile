CC = cc
CFLAGS = -Wall -Wextra -std=c11
INCLUDES = -I. -Itests

# Source files
SOURCES = main.c cli.c counter.c output.c util.c filelist.c language.c lang_defs.c strlit.c vcs.c diff.c temp_manager.c exec_helper.c archive.c parallel.c unique.c config.c
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

test_archive_runner: tests/test_archive.c tests/test_framework.c archive.c temp_manager.c exec_helper.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_archive.c tests/test_framework.c archive.c temp_manager.c exec_helper.c

test_temp_manager_runner: tests/test_temp_manager.c tests/test_framework.c temp_manager.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_temp_manager.c tests/test_framework.c temp_manager.c

test_parallel_runner: tests/test_parallel.c tests/test_framework.c parallel.c counter.c language.c lang_defs.c strlit.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_parallel.c tests/test_framework.c parallel.c counter.c language.c lang_defs.c strlit.c

test_diff_runner: tests/test_diff.c tests/test_framework.c diff.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_diff.c tests/test_framework.c diff.c

test_git_runner: tests/test_git.c tests/test_framework.c vcs.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_git.c tests/test_framework.c vcs.c

test_filter_runner: tests/test_filter.c tests/test_framework.c filelist.c util.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_filter.c tests/test_framework.c filelist.c util.c

test_util_runner: tests/test_util.c tests/test_framework.c util.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_util.c tests/test_framework.c util.c

test_vcs_runner: tests/test_vcs.c tests/test_framework.c vcs.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_vcs.c tests/test_framework.c vcs.c

test_unique_runner: tests/test_unique.c tests/test_framework.c unique.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_unique.c tests/test_framework.c unique.c

test_config_runner: tests/test_config.c tests/test_framework.c config.c cli.c util.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_config.c tests/test_framework.c config.c cli.c util.c

test_output_runner: tests/test_output.c tests/test_framework.c output.c counter.c language.c lang_defs.c strlit.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_output.c tests/test_framework.c output.c counter.c language.c lang_defs.c strlit.c

test: test_runner test_filelist_runner test_language_runner test_strlit_runner test_block_comments_runner test_continuation_runner test_archive_runner test_temp_manager_runner test_parallel_runner test_git_runner test_filter_runner test_diff_runner test_unique_runner test_config_runner test_util_runner test_vcs_runner test_output_runner
	./test_runner
	./test_filelist_runner
	./test_language_runner
	./test_strlit_runner
	./test_block_comments_runner
	./test_continuation_runner
	./test_archive_runner
	./test_temp_manager_runner
	./test_parallel_runner
	./test_git_runner
	./test_filter_runner
	./test_diff_runner
	./test_util_runner
	./test_unique_runner
	./test_config_runner
	./test_vcs_runner
	./test_output_runner

# Clean target
clean:
	rm -f rloc test_runner test_filelist_runner test_language_runner test_strlit_runner test_block_comments_runner test_continuation_runner test_archive_runner test_temp_manager_runner test_parallel_runner test_git_runner test_filter_runner test_diff_runner test_unique_runner test_config_runner test_util_runner test_vcs_runner test_output_runner

# Format target
fmt:
	clang-format -i *.c *.h tests/*.c tests/*.h

.PHONY: clean fmt test
