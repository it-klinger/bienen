################################################################################
#
# bienen
#
################################################################################

BIENEN_VERSION = 0.3.0
BIENEN_SOURCE = bienen-$(BIENEN_VERSION).tgz
BIENEN_OVERRIDE_SRCDIR = /sandboxGIT/projekte/bienen/
BIENEN_SITE = http://downloads.sourceforge.net/project/bienen/bienen/bienen-$(BIENEN_VERSION)
BIENEN_LICENSE = GPL-2.0
BIENEN_LICENSE_FILES = COPYRIGHT

BIENEN_CFLAGS = $(TARGET_CFLAGS)
BIENEN_LDFLAGS = $(TARGET_LDFLAGS)
BIENEN_DEPENDENCIES += libconfig postgresql

define BIENEN_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D) all
endef

define BIENEN_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/opt/bienen/bin
	$(INSTALL) -m 0755 -D $(@D)/bienen $(TARGET_DIR)/opt/bienen/bin/
endef

define BIENEN_INSTALL_INIT_SYSV
	$(INSTALL) -D -m 0755 $(BIENEN_PKGDIR)/S99bienen $(TARGET_DIR)/etc/init.d/S99bienen
	$(INSTALL) -D -m 0755 $(@D)/bienen.cfg $(TARGET_DIR)/etc/bienen.cfg
endef

$(eval $(generic-package))
