# SPDX-FileCopyrightText: 2022-2025 Geoffrey D. Bennett <g@b4.vu>
# SPDX-License-Identifier: GPL-3.0-or-later

# ... (previous makefile content - keep everything before the SRCS line) ...

SRCS := $(sort $(wildcard *.c) $(XML_OBJ))
OBJS := $(patsubst %.c,%.o,$(SRCS))
TARGET := alsa-scarlett-gui
DOMAIN_PREFIX := vu.b4
DESKTOP_FILE := $(DOMAIN_PREFIX).$(TARGET).desktop
ICON_FILE := $(DOMAIN_PREFIX).$(TARGET).png

GLIB_COMPILE_RESOURCES := $(shell $(PKG_CONFIG) --variable=glib_compile_resources gio-2.0)

all: $(TARGET) $(DESKTOP_FILE)

clean: depclean
	rm -f $(TARGET) $(DESKTOP_FILE) $(OBJS) $(XML_OBJ)
	$(MAKE) -C tests clean

depclean:
	rm -rf $(DEPDIR)

$(DEPDIR): ; @mkdir -p $@

DEPFILES := $(SRCS:%.c=$(DEPDIR)/%.d) $(XML_SRC:%.xml=$(DEPDIR)/%-xml.d)
$(DEPFILES):

include $(wildcard $(DEPFILES))

$(TARGET): $(OBJS)
	$(PRINTF) "$(COLOR_GREEN)[100%%]$(COLOR_RESET) $(COLOR_BLUE)LINK$(COLOR_RESET) %s\n" $@
	$(Q)$(CC) -o $(TARGET) $(OBJS) ${LDFLAGS}

ifeq ($(PREFIX),)
  PREFIX := /usr/local
endif

BINDIR := $(DESTDIR)$(PREFIX)/bin
ICONTOP := $(DESTDIR)$(PREFIX)/share/icons/hicolor
ICONDIR := $(ICONTOP)/256x256/apps
DESKTOPDIR := $(DESTDIR)$(PREFIX)/share/applications

$(DESKTOP_FILE): $(DESKTOP_FILE).template
	$(PRINTF) "$(COLOR_GREEN)[100%%]$(COLOR_RESET) $(COLOR_BLUE)GEN$(COLOR_RESET)  %s\n" $@
	$(Q)sed 's_PREFIX_$(PREFIX)_' < $< > $@

install: all
	$(PRINTF) "$(COLOR_BLUE)INSTALL$(COLOR_RESET) %s\n" $(BINDIR)/$(TARGET)
	$(Q)install -d $(BINDIR)
	$(Q)install -m 755 $(TARGET) $(BINDIR)
	$(PRINTF) "$(COLOR_BLUE)INSTALL$(COLOR_RESET) %s\n" $(ICONDIR)/$(ICON_FILE)
	$(Q)install -d $(ICONDIR)
	$(Q)install -m 644 img/$(ICON_FILE) $(ICONDIR)
	$(PRINTF) "$(COLOR_BLUE)INSTALL$(COLOR_RESET) %s\n" $(DESKTOPDIR)/$(DESKTOP_FILE)
	$(Q)install -d $(DESKTOPDIR)
	$(Q)install -m 644 $(DESKTOP_FILE) $(DESKTOPDIR)

uninstall:
	$(PRINTF) "$(COLOR_BLUE)REMOVE$(COLOR_RESET)  %s\n" $(BINDIR)/$(TARGET)
	$(Q)rm -f $(BINDIR)/$(TARGET)
	$(PRINTF) "$(COLOR_BLUE)REMOVE$(COLOR_RESET)  %s\n" $(ICONDIR)/$(ICON_FILE)
	$(Q)rm -f $(ICONDIR)/$(ICON_FILE)
	$(PRINTF) "$(COLOR_BLUE)REMOVE$(COLOR_RESET)  %s\n" $(DESKTOPDIR)/$(DESKTOP_FILE)
	$(Q)rm -f $(DESKTOPDIR)/$(DESKTOP_FILE)

test:
	$(MAKE) -C tests test

help:
	@echo "alsa-scarlett-gui"
	@echo
	@echo "This Makefile knows about:"
	@echo "  make"
	@echo "  make install"
	@echo "  make uninstall"
	@echo "  make test"
