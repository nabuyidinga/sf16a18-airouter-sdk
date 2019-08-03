#
# Copyright (C) 2010-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=sf-mm
PKG_VERSION:=1
PKG_RELEASE:=1
PKG_BUILD_DIR:= $(BUILD_DIR)/$(PKG_NAME)

SETTOP_MODE_ENABLE=n

include $(INCLUDE_DIR)/package.mk

define Package/sf-mm
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=siwifi multimedia framework
  MAINTAINER:=Franklin Wang <franklin.wang@siflower.com.cn>
  DEPENDS:=+libc +libpthread +libgstreamer1 +libwebsockets +libjson-c +alsa-lib +libubus +libblobmsg-json +aispeech
endef

define Package/sf-mm/description
Siflower multimedia framework
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	cp ./bin/* $(PKG_BUILD_DIR)
	cp ./files $(PKG_BUILD_DIR) -r
	cp ./libs $(PKG_BUILD_DIR) -r
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/sf-mm/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/share/
	$(INSTALL_DIR) $(1)/usr/share/sf-mm/
	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_DIR) $(1)/usr/lib/
	if [ "${CONFIG_TARGET_siflower_sf16a18_fullmask_SF16A18-AIR001-V1}" == "y" ] ; then \
		$(INSTALL_BIN) $(PKG_BUILD_DIR)/sf-mm-v1 $(1)/usr/sbin/sf-mm ; \
		cp $(PKG_BUILD_DIR)/files/config/sf-mm-v1 $(1)/etc/config/sf-mm ; \
	else \
		if [ "${SETTOP_MODE_ENABLE}" == "y" ] ; then \
			$(INSTALL_BIN) $(PKG_BUILD_DIR)/sf-mm-settop $(1)/usr/sbin/sf-mm ; \
			$(INSTALL_BIN) $(PKG_BUILD_DIR)/dsb $(1)/usr/sbin/dsb ; \
			rm -r $(PKG_BUILD_DIR)/files/audios/hello ; \
			cp -r $(PKG_BUILD_DIR)/files/audios/hello_settop $(PKG_BUILD_DIR)/files/audios/hello ; \
			sed -i "/network_check\.sh/d" $(PKG_BUILD_DIR)/files/sfmmd ; \
		else \
			$(INSTALL_BIN) $(PKG_BUILD_DIR)/sf-mm-v2 $(1)/usr/sbin/sf-mm ; \
			sed -i "/dsb/d" $(PKG_BUILD_DIR)/files/sfmmd ; \
		fi; \
		cp $(PKG_BUILD_DIR)/files/config/sf-mm-v2 $(1)/etc/config/sf-mm ; \
	fi
	cp $(PKG_BUILD_DIR)/files/sfmmd $(1)/etc/init.d/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cron_check $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/msleep $(1)/usr/sbin/
	cp $(PKG_BUILD_DIR)/gst-* $(1)/usr/sbin/
	cp $(PKG_BUILD_DIR)/aplay $(1)/usr/sbin/
	cp $(PKG_BUILD_DIR)/arecord $(1)/usr/sbin/
	cp $(PKG_BUILD_DIR)/amixer $(1)/usr/sbin/
	cp $(PKG_BUILD_DIR)/audio_player $(1)/usr/sbin/
	cp $(PKG_BUILD_DIR)/files/ext/script_consumer.sh $(1)/usr/sbin/
	cp $(PKG_BUILD_DIR)/files/ext/reload_acl.sh $(1)/usr/sbin/
	cp $(PKG_BUILD_DIR)/files/ext/alarm_action.sh $(1)/usr/sbin/
	cp $(PKG_BUILD_DIR)/files/ext/del_alarm.sh $(1)/usr/sbin/
	cp $(PKG_BUILD_DIR)/files/ext/wds_setup.sh $(1)/usr/sbin/
	cp $(PKG_BUILD_DIR)/files/ext/network_check.sh $(1)/usr/sbin/
	cp -r $(PKG_BUILD_DIR)/files/ext/functions $(1)/usr/share/sf-mm/
	cp -r $(PKG_BUILD_DIR)/files/alsa $(1)/usr/share/
	cp -r $(PKG_BUILD_DIR)/libs/* $(1)/usr/lib/
	cp -r $(PKG_BUILD_DIR)/files/audios $(1)/usr/share/sf-mm/audio_files/
endef

$(eval $(call BuildPackage,sf-mm))
