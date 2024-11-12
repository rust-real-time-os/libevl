# SPDX-License-Identifier: MIT

include ../config.mk
include ../libversion.mk

SONAME := libepos.so
DTSONAME := $(SONAME).$(EVL_IVERSION)
SOLIBNAME := libepos-$(EVL_SERIAL).so.$(EVL_IVERSION)
ARLIBNAME := libepos-$(EVL_SERIAL).a
WRAPPERSNAME := epos.wrappers
COMPILE_AND_LINKING_COMMAND_FILE_SUFFIX := linking_command
STATIC_LINKING_COMMAND := static_$(COMPILE_AND_LINKING_COMMAND_FILE_SUFFIX)
DYNAMIC_LINKING_COMMAND := dynamic_$(COMPILE_AND_LINKING_COMMAND_FILE_SUFFIX)
TARGETS := $(O_DIR)/$(SOLIBNAME) $(O_DIR)/$(ARLIBNAME)


SRCFILES := $(wildcard *.c)
PIC_OBJFILES = $(SRCFILES:%.c=$(O_DIR)/%-pic.o)
OBJFILES = $(SRCFILES:%.c=$(O_DIR)/%.o)
DEPFILES = $(SRCFILES:%.c=$(O_DIR)/%.d)

LIB_CPPFLAGS :=  $(BASE_CPPFLAGS)		\
		 -I.				\
		 -I../include			\
		 -Iarch/$(ARCH)/include		\
		 -I$(O_DIR)/../include		\
		 -I$(O_DIR)

LIB_CFLAGS := $(LIB_CPPFLAGS) $(BASE_CFLAGS) -idirafter ../include/placeholders

override CFLAGS := $(LIB_CFLAGS) $(CFLAGS)

override LDFLAGS := $(LDFLAGS) -lpthread -lrt

all: output-Makefile $(DEPFILES) $(TARGETS)

define install_linking_command
@echo '-I. -I$(DESTDIR)/include -fasynchronous-unwind-tables -Wl,-Wl,--wrap=main $(DESTDIR)/lib/libepos-0.a $(DESTDIR)/lib/libevl-0.a -lpthread -lrt -static' > $(DESTDIR)/$(libdir)/$(STATIC_LINKING_COMMAND)
@echo '-I. -I$(DESTDIR)/include -fasynchronous-unwind-tables -Wl,-Wl,--wrap=main -L$(LIBEVL_RROS)/lib -lepos -levl -lpthread -lrt' > $(DESTDIR)/$(libdir)/$(DYNAMIC_LINKING_COMMAND)
endef

define alias_static_library
ln -s $(DESTDIR)/lib/libepos-0.a $(DESTDIR)/lib/libepos.a
ln -s $(DESTDIR)/lib/libevl-0.a $(DESTDIR)/lib/libevl.a
endef

install install_all: all
	$(call inst-cmd,$(SOLIBNAME),$(INSTALL) -D $(O_DIR)/$(SOLIBNAME) -t $(DESTDIR)/$(libdir))
	@$(LN_S) $(SOLIBNAME) $(DESTDIR)/$(libdir)/$(DTSONAME)
	@$(LN_S) $(DTSONAME) $(DESTDIR)/$(libdir)/$(SONAME)
	$(call inst-cmd,$(ARLIBNAME),$(INSTALL) -D $(O_DIR)/$(ARLIBNAME) -t $(DESTDIR)/$(libdir))
	$(call inst-cmd,$(WRAPPERSNAME),$(CP) $(WRAPPERSNAME) $(DESTDIR)/$(libdir)/$(WRAPPERSNAME))
	$(call inst-cmd,$(COMPILE_AND_LINKING_COMMAND_FILE_SUFFIX),$(call install_linking_command))
	$(call inst-cmd,"alias_static_library",$(call alias_static_library))


clean clobber mrproper: output-Makefile
	$(Q)$(RM) -f $(O_DIR)/git-stamp.h $(PIC_OBJFILES) $(OBJFILES) \
		$(TARGETS) $(O_DIR)/$(DTSONAME) $(DEPFILES)

$(O_DIR)/$(SOLIBNAME): $(PIC_OBJFILES)
	$(call ld-cmd,$@,$(CC) -shared -Wl$(comma)-soname$(comma)$(DTSONAME) -o $(@) \
		$(PIC_OBJFILES) $(LDFLAGS) -Wl$(comma)-rpath=$(libdir) -Wl$(comma)--export-dynamic)
	$(Q)$(LN_S) $(SOLIBNAME) $(O_DIR)/$(DTSONAME)

$(O_DIR)/$(ARLIBNAME): $(OBJFILES)
	$(call ar-cmd,$@,$(AR) ru $@ $(OBJFILES))

# Generate
$(O_DIR)/%-pic.o: %.c $(O_DIR)/%.d
	$(call cc-pic-cmd,$@,$(CC) $(CFLAGS) -fPIC -c -o $@ $<)

# Generate dependency files
$(O_DIR)/%.o: %.c $(O_DIR)/%.d
	$(call cc-cmd,$@,$(CC) $(CFLAGS) -c -o $@ $<)

-include $(DEPFILES)
