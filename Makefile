CC = cc
CFLAGS = -Wall -Wextra -std=c11
INCLUDES = -I. -Itests
BINDIR = bin

# Default target
build: bin/rloc

# Ensure bin directory exists
$(BINDIR):
	mkdir -p $(BINDIR)

# Source files
SOURCES = main.c cli.c counter.c output.c util.c filelist.c language.c lang_defs.c strlit.c vcs.c diff.c temp_manager.c exec_helper.c archive.c parallel.c unique.c config.c
OBJECTS = $(SOURCES:.c=.o)

# Main target
bin/rloc: $(SOURCES) | $(BINDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(SOURCES)

# Test targets
bin/test_runner: tests/test_counter.c tests/test_framework.c counter.c strlit.c | $(BINDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_counter.c tests/test_framework.c counter.c strlit.c

bin/test_filelist_runner: tests/test_filelist.c tests/test_framework.c filelist.c util.c | $(BINDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_filelist.c tests/test_framework.c filelist.c util.c

bin/test_language_runner: tests/test_language.c tests/test_framework.c language.c lang_defs.c | $(BINDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_language.c tests/test_framework.c language.c lang_defs.c

bin/test_strlit_runner: tests/test_strlit.c tests/test_framework.c strlit.c | $(BINDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_strlit.c tests/test_framework.c strlit.c

bin/test_block_comments_runner: tests/test_block_comments.c tests/test_framework.c counter.c strlit.c language.c lang_defs.c | $(BINDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_block_comments.c tests/test_framework.c counter.c strlit.c language.c lang_defs.c

bin/test_continuation_runner: tests/test_continuation.c tests/test_framework.c counter.c strlit.c language.c lang_defs.c | $(BINDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_continuation.c tests/test_framework.c counter.c strlit.c language.c lang_defs.c

bin/test_archive_runner: tests/test_archive.c tests/test_framework.c archive.c temp_manager.c exec_helper.c | $(BINDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_archive.c tests/test_framework.c archive.c temp_manager.c exec_helper.c

bin/test_temp_manager_runner: tests/test_temp_manager.c tests/test_framework.c temp_manager.c | $(BINDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_temp_manager.c tests/test_framework.c temp_manager.c

bin/test_parallel_runner: tests/test_parallel.c tests/test_framework.c parallel.c counter.c language.c lang_defs.c strlit.c | $(BINDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_parallel.c tests/test_framework.c parallel.c counter.c language.c lang_defs.c strlit.c

bin/test_diff_runner: tests/test_diff.c tests/test_framework.c diff.c util.c | $(BINDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_diff.c tests/test_framework.c diff.c util.c

bin/test_git_runner: tests/test_git.c tests/test_framework.c vcs.c util.c | $(BINDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_git.c tests/test_framework.c vcs.c util.c

bin/test_filter_runner: tests/test_filter.c tests/test_framework.c filelist.c util.c | $(BINDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_filter.c tests/test_framework.c filelist.c util.c

bin/test_util_runner: tests/test_util.c tests/test_framework.c util.c | $(BINDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_util.c tests/test_framework.c util.c

bin/test_vcs_runner: tests/test_vcs.c tests/test_framework.c vcs.c util.c | $(BINDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_vcs.c tests/test_framework.c vcs.c util.c

bin/test_unique_runner: tests/test_unique.c tests/test_framework.c unique.c | $(BINDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_unique.c tests/test_framework.c unique.c

bin/test_config_runner: tests/test_config.c tests/test_framework.c config.c cli.c util.c | $(BINDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_config.c tests/test_framework.c config.c cli.c util.c

bin/test_output_runner: tests/test_output.c tests/test_framework.c output.c counter.c language.c lang_defs.c strlit.c | $(BINDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_output.c tests/test_framework.c output.c counter.c language.c lang_defs.c strlit.c

bin/test_phase4_runner: tests/test_phase4.c tests/test_framework.c cli.c | $(BINDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ tests/test_phase4.c tests/test_framework.c cli.c

test: bin/test_runner bin/test_filelist_runner bin/test_language_runner bin/test_strlit_runner bin/test_block_comments_runner bin/test_continuation_runner bin/test_archive_runner bin/test_temp_manager_runner bin/test_parallel_runner bin/test_git_runner bin/test_filter_runner bin/test_diff_runner bin/test_unique_runner bin/test_config_runner bin/test_util_runner bin/test_vcs_runner bin/test_output_runner bin/test_phase4_runner
	$(BINDIR)/test_runner
	$(BINDIR)/test_filelist_runner
	$(BINDIR)/test_language_runner
	$(BINDIR)/test_strlit_runner
	$(BINDIR)/test_block_comments_runner
	$(BINDIR)/test_continuation_runner
	$(BINDIR)/test_archive_runner
	$(BINDIR)/test_temp_manager_runner
	$(BINDIR)/test_parallel_runner
	$(BINDIR)/test_git_runner
	$(BINDIR)/test_filter_runner
	$(BINDIR)/test_diff_runner
	$(BINDIR)/test_util_runner
	$(BINDIR)/test_unique_runner
	$(BINDIR)/test_config_runner
	$(BINDIR)/test_vcs_runner
	$(BINDIR)/test_output_runner
	$(BINDIR)/test_phase4_runner

# Clean target
clean:
	rm -rf $(BINDIR)

# Format target
fmt:
	clang-format -i *.c *.h tests/*.c tests/*.h

.PHONY: build clean fmt test
