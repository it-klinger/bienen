################################################################################
#
# pqbienen
#
################################################################################

PQBIENEN_VERSION = 0.3.0
PQBIENEN_SOURCE = pqbienen-$(PQBIENEN_VERSION).tgz
PQBIENEN_OVERRIDE_SRCDIR = /sandboxGIT/projekte/postgresql-bienen/
PQBIENEN_SITE = http://downloads.sourceforge.net/project/pqbienen/pqbienen/pqbienen-$(PQBIENEN_VERSION)
PQBIENEN_LICENSE = GPL-2.0
PQBIENEN_LICENSE_FILES = COPYRIGHT

#PQBIENEN_CFLAGS = $(TARGET_CFLAGS)
#PQBIENEN_LDFLAGS = $(TARGET_LDFLAGS)
# PQBIENEN_DEPENDENCIES += libconfig postgresql

#define PQBIENEN_BUILD_CMDS
#	$(TARGET_MAKE_ENV) $(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D) all
#endef

define PQBIENEN_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/opt/pqbienen/sql
	$(INSTALL) -m 0755 -D $(@D)/bw.sql $(TARGET_DIR)/opt/pqbienen/sql/
endef

#define PQBIENEN_INSTALL_INIT_SYSV
#	$(INSTALL) -D -m 0755 $(PQBIENEN_PKGDIR)/S99pqbienen $(TARGET_DIR)/etc/init.d/S99pqbienen
#	$(INSTALL) -D -m 0755 $(PQBIENEN_PKGDIR)/pqbienen.cfg $(TARGET_DIR)/etc/pqbienen.cfg
#endef

$(eval $(generic-package))
