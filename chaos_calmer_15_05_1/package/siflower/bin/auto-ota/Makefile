#
# Copyright (C) 2010-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=auto-ota
PKG_VERSION:=1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/auto-ota
  SECTION:=net
  CATEGORY:=Network
  TITLE:=auto-ota
  MAINTAINER:=Xijun.guo <xijun.guo@siflower.com.cn>
endef

define Package/auto-ota/description
auto ota
endef

define Build/Compile/Default

endef

define Package/auto-ota/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) ./files/auto_ota $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,auto-ota))
