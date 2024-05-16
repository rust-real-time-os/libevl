# SPDX-License-Identifier: MIT

include config.mk

TARGETS := include lib benchmarks utils eshi tests tidbits epos

$(MAIN_GOALS): output-Makefile
	@for target in $(TARGETS); do						\
		$(MAKE) -C $$target O=$(O_DIR)/$$target V=$(V) $@ || exit 1;	\
	done

lib: include

epos tests benchmarks utils: lib
