--
--[[
LuCI - Lua Configuration Interface

Description:
Offers an interface for handle app request
]]--

module("luci.controller.api.sfsystem", package.seeall)

local sysutil = require "luci.siwifi.sf_sysutil"
local sysconfig = require "luci.siwifi.sf_sysconfig"
local disk = require "luci.siwifi.sf_disk"
local sferr = require "luci.siwifi.sf_error"
local wirelessnew = require "luci.controller.admin.wirelessnew"
local nixio = require "nixio"
local fs = require "nixio.fs"
local json = require("luci.json")
local http = require "luci.http"
local uci = require "luci.model.uci"
local _uci_real  = cursor or _uci_real or uci.cursor()
local ap = nil
local nixio = require "nixio"
local json = require("luci.json")
local deviceImpl = require("luci.siwifi.deviceImpl")
local networkImpl = require("luci.siwifi.networkImpl")
local wirelessImpl = require("luci.siwifi.wirelessImpl")

function index()
    local page   = node("api","sfsystem")
    page.target  = firstchild()
    page.title   = ("")
    page.order   = 100
    page.sysauth = "admin"
    page.sysauth_authenticator = "jsonauth"
    page.index = true
    entry({"api", "sfsystem"}, firstchild(), (""), 100)
    entry({"api", "sfsystem", "welcome"}, call("welcome"), nil)
    entry({"api", "sfsystem", "init_info"}, call("init_info"), nil)

    page = entry({"api", "sfsystem", "command"}, call("command"), nil)
    page.leaf = true

    entry({"api", "sfsystem", "get_stok_local"}, call("get_stok_local"), nil)
    entry({"api", "sfsystem", "get_stok_remote"}, call("get_stok_remote"), nil)
    entry({"api", "sfsystem", "setpasswd"}, call("setpasswd"), nil)
    entry({"api", "sfsystem", "wifi_detail"}, call("wifi_detail"),nil)
    entry({"api", "sfsystem", "setwifi"}, call("setwifi"),nil)
    entry({"api", "sfsystem", "getwifi_advanced"}, call("getwifi_advanced"),nil)
    entry({"api", "sfsystem", "setwifi_advanced"}, call("setwifi_advanced"),nil)
    entry({"api", "sfsystem", "main_status"}, call("main_status"),nil)
    entry({"api", "sfsystem", "bind"}, call("bind"),nil)
    entry({"api", "sfsystem", "unbind"}, call("unbind"),nil)
    entry({"api", "sfsystem", "manager"}, call("manager"),nil)
    entry({"api", "sfsystem", "device_list_backstage"}, call("device_list_backstage"),nil)           --just internal call
    entry({"api", "sfsystem", "arp_check_dev"}, call("arp_check_dev"),nil)           --just internal call
    entry({"api", "sfsystem", "device_list"}, call("device_list"),nil)
    entry({"api", "sfsystem", "del_device_info"}, call("del_device_info"),nil)
    entry({"api", "sfsystem", "setdevice"}, call("setdevice"),nil)
    entry({"api", "sfsystem", "ota_check"}, call("ota_check"),nil)
    entry({"api", "sfsystem", "ota_check2"}, call("ota_check2"),nil)
    entry({"api", "sfsystem", "ac_ap_ota_check"}, call("ac_ap_ota_check"),nil)
    entry({"api", "sfsystem", "ota_upgrade"}, call("ota_upgrade"),nil)
    entry({"api", "sfsystem", "ac_ota_upgrade"}, call("ac_ota_upgrade"),nil)
    entry({"api", "sfsystem", "check_wan_type"}, call("check_wan_type"),nil)
    entry({"api", "sfsystem", "get_wan_type"}, call("get_wan_type"),nil)
    entry({"api", "sfsystem", "set_wan_type"}, call("set_wan_type"),nil)
    entry({"api", "sfsystem", "get_lan_type"}, call("get_lan_type"),nil)
    entry({"api", "sfsystem", "set_lan_type"}, call("set_lan_type"),nil)
    entry({"api", "sfsystem", "detect_wan_type"}, call("detect_wan_type"),nil)
    entry({"api", "sfsystem", "qos_set"}, call("qos_set"),nil)
    entry({"api", "sfsystem", "qos_info"}, call("qos_info"),nil)
    entry({"api", "sfsystem", "netdetect"}, call("netdetect"),nil)
    entry({"api", "sfsystem", "check_net"}, call("check_net"),nil)
    entry({"api", "sfsystem", "set_wifi_filter"}, call("set_wifi_filter"),nil)
    entry({"api", "sfsystem", "get_wifi_filter"}, call("get_wifi_filter"),nil)
    entry({"api", "sfsystem", "upload_log"}, call("upload_log"),nil)
    entry({"api", "sfsystem", "sync"}, call("sync"),nil)
    entry({"api", "sfsystem", "download"}, call("download"),nil)
    entry({"api", "sfsystem", "update_qos_local"}, call("update_qos_local"),nil)                            --just internal call
    entry({"api", "sfsystem", "set_user_info"}, call("set_user_info"), nil)
    entry({"api", "sfsystem", "new_oray_params"}, call("new_oray_params"), nil)
    entry({"api", "sfsystem", "destroy_oray_params"}, call("destroy_oray_params"), nil)
    entry({"api", "sfsystem", "setdefault"}, call("setdefault"), nil)
    entry({"api", "sfsystem", "getdefault"}, call("getdefault"), nil)
    entry({"api", "sfsystem", "adduser"}, call("adduser"), nil)
    entry({"api", "sfsystem", "setdevicetime"}, call("setdevicetime"), nil)
    entry({"api", "sfsystem", "getdevicetime"}, call("getdevicetime"), nil)
    entry({"api", "sfsystem", "setdevicerestrict"}, call("setdevicerestrict"), nil)
    entry({"api", "sfsystem", "getdevicerestrict"}, call("getdevicerestrict"), nil)
    entry({"api", "sfsystem", "setdevicedatausage"}, call("setdevicedatausage"), nil)
    entry({"api", "sfsystem", "getdevicedatausage"}, call("getdevicedatausage"), nil)
    entry({"api", "sfsystem", "routerlivetime"}, call("routerlivetime"), nil)
    entry({"api", "sfsystem", "blockrefactory"}, call("blockrefactory"), nil)
    entry({"api", "sfsystem", "getrouterlivetime"}, call("get_routerlivetime"), nil)
    entry({"api", "sfsystem", "getblockrefactory"}, call("getblockrefactory"), nil)
    entry({"api", "sfsystem", "getaccess"}, call("getaccess"), nil)

    entry({"api", "sfsystem", "setspeed"}, call("setspeed"), nil)
    entry({"api", "sfsystem", "urllist_set"}, call("urllist_set"), nil)
    entry({"api", "sfsystem", "urllist_get"}, call("urllist_get"), nil)
    entry({"api", "sfsystem", "urllist_enable"}, call("urllist_enable"), nil)
    entry({"api", "sfsystem", "get_customer_wifi_iface"}, call("get_customer_wifi_iface"), nil)
    entry({"api", "sfsystem", "set_customer_wifi_iface"}, call("set_customer_wifi_iface"), nil)
    entry({"api", "sfsystem", "wifi_scan"}, call("wifi_scan"), nil)
    entry({"api", "sfsystem", "wifi_connect"}, call("wifi_connect"), nil)
    entry({"api", "sfsystem", "wds_getwanip"}, call("wds_getwanip"), nil)
    entry({"api", "sfsystem", "wds_getrelip"}, call("wds_getrelip"), nil)
    entry({"api", "sfsystem", "wds_enable"}, call("wds_enable"), nil)
    entry({"api", "sfsystem", "wds_disable"}, call("wds_disable"), nil)
    entry({"api", "sfsystem", "get_wds_info"}, call("get_wds_info"), nil)
    entry({"api", "sfsystem", "wds_sta_is_disconnected"}, call("wds_sta_is_disconnected"), nil)
    entry({"api", "sfsystem", "set_warn"}, call("set_warn"), nil)
    entry({"api", "sfsystem", "get_warn"}, call("get_warn"), nil)
    entry({"api", "sfsystem", "set_dev_warn"}, call("set_dev_warn"), nil)

    entry({"api", "sfsystem", "set_lease_net"}, call("set_lease_net"), nil)
    entry({"api", "sfsystem", "get_lease_net"}, call("get_lease_net"), nil)
    entry({"api", "sfsystem", "set_lease_mac"}, call("set_lease_mac"), nil)
    entry({"api", "sfsystem", "getrouterfeature"}, call("getrouterfeature"), nil)
    --for local useage
    entry({"api", "sfsystem", "pctl_url_check"}, call("pctl_url_check"),nil)

    entry({"api", "sfsystem", "get_ap_groups"}, call("get_ap_groups"), nil)
    entry({"api", "sfsystem", "set_ap_group"}, call("set_ap_group"), nil)
    entry({"api", "sfsystem", "remove_ap_group"}, call("remove_ap_group"), nil)
    entry({"api", "sfsystem", "get_ap_list"}, call("get_ap_list"), nil)
    entry({"api", "sfsystem", "set_ap"}, call("set_ap"), nil)
    entry({"api", "sfsystem", "delete_ap"}, call("delete_ap"), nil)

    --V17
    entry({"api", "sfsystem", "get_freq_intergration"}, call("get_freq_intergration"), nil)
    entry({"api", "sfsystem", "set_freq_intergration"}, call("set_freq_intergration"), nil)
    --V18
    entry({"api", "sfsystem", "func_adapter"}, call("func_adapter"), nil)
    entry({"api", "sfsystem", "set_samba"}, call("set_samba"), nil)
    entry({"api", "sfsystem", "get_samba"}, call("get_samba"), nil)
    entry({"api", "sfsystem", "get_alarm"}, call("get_alarm"), nil)
    entry({"api", "sfsystem", "del_alarm"}, call("del_alarm"), nil)
    entry({"api", "sfsystem", "add_alarm"}, call("add_alarm"), nil)
    entry({"api", "sfsystem", "get_volume"}, call("get_volume"), nil)
    entry({"api", "sfsystem", "set_volume"}, call("set_volume"), nil)
    entry({"api", "sfsystem", "get_sleep"}, call("get_sleep"), nil)
    entry({"api", "sfsystem", "set_sleep"}, call("set_sleep"), nil)
    entry({"api", "sfsystem", "get_music_info"}, call("get_music_info"), nil)
    entry({"api", "sfsystem", "media_control"}, call("media_control"), nil)
    entry({"api", "sfsystem", "get_media_states"}, call("get_media_states"), nil)
    entry({"api", "sfsystem", "add_to_collection"}, call("add_to_collection"), nil)
    entry({"api", "sfsystem", "get_collection"}, call("get_collection"), nil)
    entry({"api", "sfsystem", "cancel_collection"}, call("cancel_collection"), nil)
    entry({"api", "sfsystem", "media_play"}, call("media_play"), nil)
    entry({"api", "sfsystem", "get_security_mode"}, call("get_security_mode"), nil)
    entry({"api", "sfsystem", "set_security_mode"}, call("set_security_mode"), nil)
	--[[
    entry({"api", "sfsystem", "get_green_mode_enable"}, call("get_green_mode_enable"), nil)
    entry({"api", "sfsystem", "set_green_mode_enable"}, call("set_green_mode_enable"), nil)
    entry({"api", "sfsystem", "get_green_mode_config"}, call("get_green_mode_config"), nil)
    entry({"api", "sfsystem", "set_green_mode_config"}, call("set_green_mode_config"), nil)
    entry({"api", "sfsystem", "get_green_mode_device"}, call("get_green_mode_device"), nil)
    entry({"api", "sfsystem", "remove_green_mode_device"}, call("remove_green_mode_device"), nil)
	]]--
    entry({"api", "sfsystem", "get_green"}, call("get_green"), nil)
    entry({"api", "sfsystem", "set_green"}, call("set_green"), nil)
end
-- send to ssst so ssst could polling update status and send to app
-- for sure app get progress of download
function sync()
    --string.format("Downloading %s from %s to %s", file, host, outfile)
    --    local cmd = "SYNC -data "..luci.http.formvalue("enable")

    local code = 0
    local result = {}
    code,result = networkImpl.sync(get_arg_list())
    sysutil.set_easy_return(code,result)
end

function check_wan_type()
    local arg_list_table = get_arg_list()
    local result = {}
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = networkImpl.check_wan_type()
    end
    sysutil.set_easy_return(code,result)
    return
end

function check_net()
    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = networkImpl.check_net()
    end
    sysutil.set_easy_return(code,result)

end

function new_oray_params()
    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V12")

    if code  == 0 then
        code,result = networkImpl.new_oray_params()
    end
    sysutil.set_easy_return(code,result)

end

function destroy_oray_params()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V12")

    if code  == 0 then
        code = networkImpl.destroy_oray_params(arg_list_table)
    end
    sysutil.set_easy_return(code,nil)

end

function setdefault()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code = deviceImpl.setdefault(arg_list_table)
    end
    sysutil.set_easy_return(code,nil)
end

function getdefault()
    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = deviceImpl.getdefault()
    end
    sysutil.set_easy_return(code,result)

end

function getdevicetime()
    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = deviceImpl.getdevicetime(arg_list_table)
    end
    sysutil.set_easy_return(code,result)
end

function setdevicetime()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code = deviceImpl.setdevicetime(arg_list_table)
    end
    sysutil.set_easy_return(code,nil)
end

function getdevicerestrict()
    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = networkImpl.getdevicerestrict(arg_list_table)
    end
    sysutil.set_easy_return(code,result)
end

function setdevicerestrict()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code = networkImpl.setdevicerestrict(arg_list_table)
    end
    sysutil.set_easy_return(code,nil)

end

function setdevicedatausage()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code = deviceImpl.setdevicedatausage(arg_list_table)
    end
    sysutil.set_easy_return(code,nil)
end

function getdevicedatausage()
    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = deviceImpl.getdevicedatausage(arg_list_table)
    end
    sysutil.set_easy_return(code,result)
end

function get_routerlivetime()
    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = networkImpl.get_routerlivetime()
    end
    sysutil.set_easy_return(code,result)

end
function routerlivetime()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code = networkImpl.routerlivetime(arg_list_table)
    end
    sysutil.set_easy_return(code,nil)
end
function getblockrefactory()
    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = networkImpl.getblockrefactory()
    end
    sysutil.set_easy_return(code,result)
end

function blockrefactory()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code = networkImpl.blockrefactory(arg_list_table)
    end
    sysutil.set_easy_return(code,nil)
end

function getaccess()
    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = deviceImpl.getaccess(arg_list_table)
    end
    sysutil.set_easy_return(code,result)
end

function adduser()
    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = deviceImpl.adduser(arg_list_table)
    end
    sysutil.set_easy_return(code, result)
end

function bind()
    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = deviceImpl.bind(arg_list_table)
    end
    sysutil.set_easy_return(code, result)
end

function manager()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code = deviceImpl.manager(arg_list_table)
    end
    sysutil.set_easy_return(code, nil)
end

function unbind()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code = deviceImpl.unbind(arg_list_table)
    end
    sysutil.sflog("INFO","Router unbind!")
    sysutil.set_easy_return(code, nil)
end

function init_info()
    local arg_list_table = get_arg_list()
    local result = {}
    local code = protocol_check(arg_list_table["version"], "V10")
    if code  == 0 then
        code, result = networkImpl.init_info()
    end
    sysutil.set_easy_return(code, result)
end

function command()
    local arg_list_table = get_arg_list()
    local result = {}
    local code = protocol_check(arg_list_table["version"], "V10")
    if code  == 0 then
        code, result = networkImpl.command(arg_list_table)
    end
    sysutil.set_easy_return(code, result)
end

function get_stok_local()
    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")
    if code  == 0 then
        code,result = deviceImpl.get_stok_local()
    end
    sysutil.set_easy_return(code, result)
end

function get_stok_remote()
    --return the same value as local request
    get_stok_local()
end

function setpasswd(arg_list_table)
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code = deviceImpl.setpasswd(arg_list_table)
    end
    sysutil.set_easy_return(code, nil)
    return
end


function wifi_detail()
    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = wirelessImpl.wifi_detail(arg_list_table)
    end
    sysutil.set_easy_return(code, result)
    return
end

function get_wan_type()
    local arg_list_table = get_arg_list()
    local result = {}
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = networkImpl.get_wan_type()
    end
    sysutil.set_easy_return(code, result)
    return
end

function set_wan_type()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code = networkImpl.set_wan_type(arg_list_table)
    end
    sysutil.set_easy_return(code, nil)
    return
end

function get_lan_type()
    local arg_list_table = get_arg_list()
    local result = {}
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = networkImpl.get_lan_type()
    end
    sysutil.set_easy_return(code, result)
    return
end

function set_lan_type()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code = networkImpl.set_lan_type(arg_list_table)
    end
    sysutil.set_easy_return(code, nil)
    return
end

function detect_wan_type()
    local arg_list_table = get_arg_list()
    local result = {}
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = networkImpl.detect_wan_type()
    end
    sysutil.set_easy_return(code, result)
    return
end


function setwifi()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")
    if code  == 0 then
        code = wirelessImpl.setwifi(arg_list_table)
    end
    sysutil.sflog("INFO","Wifi configure changed!")
    sysutil.set_easy_return(code, nil)
    return

end

function setwifi_advanced()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code = wirelessImpl.setwifi_advanced(arg_list_table)
    end
    sysutil.sflog("INFO","Advanced wifi configure changed!")
    sysutil.set_easy_return(code, nil)
    return
end

function getwifi_advanced()
    local arg_list_table = get_arg_list()
    local result = {}
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = wirelessImpl.getwifi_advanced()
    end
    sysutil.set_easy_return(code, result)
    return
end

function get_customer_wifi_iface()

    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = wirelessImpl.get_customer_wifi_iface()
    else
        sysutil.set_easy_return(code, result)
    end
    sysutil.set_easy_return(code, result)
    return
end

function set_customer_wifi_iface()

    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = wirelessImpl.set_customer_wifi_iface(arg_list_table)
    else
        sysutil.set_easy_return(code, result)
    end
    sysutil.set_easy_return(code, result)
    --	sysutil.sflog("INFO","Guest wifi configure changed!"%{mac})--customer wifi configure change!的log是用网页打印的
    return
end

function wifi_scan()

    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = wirelessImpl.wifi_scan(arg_list_table)
    else
        sysutil.set_easy_return(code, result)
    end
    sysutil.set_easy_return(code, result)
    return
end

function wifi_connect()

    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = wirelessImpl.wifi_connect(arg_list_table)
    else
        sysutil.set_easy_return(code, result)
    end
    sysutil.set_easy_return(code, result)
    return
end

function wds_getrelip()

    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V17")

    if code == 0 then
        code,result = wirelessImpl.wds_getrelip(arg_list_table)
    else
        sysutil.set_easy_return(code, result)
    end
    sysutil.set_easy_return(code, result)
    return
end

function wds_getwanip()

    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = wirelessImpl.wds_getwanipimpl(arg_list_table)
    end
    sysutil.set_easy_return(code, result)

    return
end

function wds_enable()

    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = wirelessImpl.wds_enable(arg_list_table)
    else
        sysutil.set_easy_return(code, result)
    end
    sysutil.set_easy_return(code, result)
    return
end

function wds_disable()

    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = wirelessImpl.wds_disable(arg_list_table)
    else
        sysutil.set_easy_return(code, result)
    end
    sysutil.set_easy_return(code, result)
    return
end

function get_wds_info()

    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = wirelessImpl.get_wds_info()
    else
        sysutil.set_easy_return(code, result)
    end
    sysutil.set_easy_return(code, result)
    return
end

function wds_sta_is_disconnected()
	local arg_list_table = get_arg_list()
	local code = protocol_check(arg_list_table["version"], "V18")

	if code  == 0 then
		code = wirelessImpl.wds_sta_is_disconnected()
	end
	sysutil.set_easy_return(code, nil)
end

function main_status()
    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = deviceImpl.main_status(arg_list_table)
    end
    sysutil.set_easy_return(code, result)
    return
end
function protocol_check(protocol, version )
    local code = 0
    if(not protocol) then
        code = sferr.ERROR_NO_PROTOCOL_NOT_FOUND
    elseif( not sysutil.version_check(protocol) or  protocol < version) then
        code = sferr.ERROR_NO_PROTOCOL_NOT_SUPPORT
    end
    return code
end

function setspeed()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V12")

    if code  == 0 then
        code = networkImpl.setspeed(arg_list_table)
    end
    sysutil.set_easy_return(code, nil)
    return
end

function urllist_set()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V12")

    if code  == 0 then
        code = networkImpl.urllist_set(arg_list_table)
    end
    sysutil.set_easy_return(code,nil)
    return
end

function urllist_get()

    local result = { }
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V12")

    if code  == 0 then
        code,result = networkImpl.urllist_get(arg_list_table)
    end
    sysutil.set_easy_return(code,result)
    return
end

function urllist_enable()

    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V12")

    if code  == 0 then
        code = networkImpl.urllist_enable(arg_list_table)
    end
    sysutil.set_easy_return(code,nil);
    return
end

function arp_check_dev()
    local arg_list_table = get_arg_list()
    local result = {}
    local code = 0
    code,result = networkImpl.arp_check_dev(arg_list_table)
    sysutil.set_easy_return(code, result)
end

function device_list_backstage()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code = networkImpl.device_list_backstage()
    end
    sysutil.set_easy_return(code, nil)
    return

end

function device_list()
    local arg_list_table = get_arg_list()
    local result = {}
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = deviceImpl.device_list(arg_list_table)
    end
    sysutil.set_easy_return(code, result)
    return
end

function del_device_info()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code = deviceImpl.del_device_info(arg_list_table)
    end
    sysutil.set_easy_return(code, nil)
    return
end

function update_qos_local()
    --    update_qos()
    code = 0
    code = networkImpl.update_qos_local()
    sysutil.set_easy_return(code, nil)
    return
end

function setdevice()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V12")

    if code  == 0 then
        code = deviceImpl.setdevice(arg_list_table)
    end
    sysutil.set_easy_return(code, nil)
    return
end

function qos_set()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V12")

    if code  == 0 then
        code = networkImpl.qos_set(arg_list_table)
    end
    sysutil.set_easy_return(code, nil)
    return
end

function qos_info()
    local arg_list_table = get_arg_list()
    local result = {}
    local code = protocol_check(arg_list_table["version"], "V12")

    if code  == 0 then
        code,result = networkImpl.qos_info()
    end
    sysutil.set_easy_return(code, result)
    return
end

function welcome()
    local arg_list_table = get_arg_list()
    local result = {}
    local code = protocol_check(arg_list_table["version"], "V12")

    if code  == 0 then
        code,result = networkImpl.welcome()
    end
    sysutil.set_easy_return(code, result)
end

function ota_check()
    local arg_list_table = get_arg_list()
    local result = {}
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = networkImpl.ota_check()
    end
    sysutil.set_easy_return(code, result)
    return
end

function ota_check2()
    local arg_list_table = get_arg_list()
    local result = {}
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = networkImpl.ota_check2()
    end
    sysutil.set_easy_return(code, result)
    return
end

function ac_ap_ota_check()
    local arg_list_table = get_arg_list()
    local result = {}
    local code = protocol_check(arg_list_table["version"], "V12")

    if code  == 0 then
        code,result = networkImpl.ac_ap_ota_check()
    end
    sysutil.set_easy_return(code, result)
    return
end

function ota_upgrade()
    local arg_list_table = get_arg_list()
    local result = {}
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = networkImpl.ota_upgrade(arg_list_table)
    end
    sysutil.set_easy_return(code, result)
    return
end

function ac_ota_upgrade()
    local arg_list_table = get_arg_list()
    local result = {}
    local code = protocol_check(arg_list_table["version"], "V15")

    if code  == 0 then
        code,result = networkImpl.ac_ota_upgrade(arg_list_table)
    end
    sysutil.set_easy_return(code, result)
    return
end

function netdetect()
    local arg_list_table = get_arg_list()
    local result = {}
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = networkImpl.netdetect(arg_list_table)
    end
    sysutil.set_easy_return(code, result)
end

function set_wifi_filter()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code = deviceImpl.set_wifi_filter(arg_list_table)
    end
    sysutil.set_easy_return(code, nil)
end

function get_wifi_filter()
    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = deviceImpl.get_wifi_filter()
    end
    sysutil.set_easy_return(code, result)

end

function upload_log()
    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = networkImpl.upload_log(arg_list_table)
    end
    sysutil.set_easy_return(code, result)
end

function download()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code = networkImpl.download(arg_list_table)
    end
    sysutil.set_easy_return(code, nil)
end

function set_user_info()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code = deviceImpl.set_user_info(arg_list_table)
    end
    sysutil.set_easy_return(code, nil)
end

function set_warn()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code = deviceImpl.set_warn(arg_list_table)
    end
    sysutil.set_easy_return(code, nil)
end

function get_warn()
    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = deviceImpl.get_warn()
    end
    sysutil.set_easy_return(code, result)
end

function set_dev_warn()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code = deviceImpl.set_dev_warn(arg_list_table)
    end
    sysutil.set_easy_return(code, nil)
end

function set_lease_net()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V14")

    if code  == 0 then
        code = deviceImpl.set_lease_net(arg_list_table)
    end
    sysutil.set_easy_return(code, nil)
end

function get_lease_net()
    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V14")

    if code  == 0 then
        code,result = deviceImpl.get_lease_net()
    end
    sysutil.set_easy_return(code, result)
end

function set_lease_mac()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V14")

    if code  == 0 then
        code = deviceImpl.set_lease_mac(arg_list_table)
    end
    sysutil.set_easy_return(code, nil)
end

-- internal interface for ssst
function getrouterfeature()
    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V10")

    if code  == 0 then
        code,result = networkImpl.getrouterfeature(arg_list_table)
    end
    sysutil.set_easy_return(code, result)
end

function get_ap_groups()
    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V16")
    if code  == 0 then
        code,result = networkImpl.get_ap_groups()
    end
    sysutil.set_easy_return(code, result)
end

function set_ap_group()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V16")
    if code  == 0 then
        code = networkImpl.set_ap_group(arg_list_table)
    end
    sysutil.set_easy_return(code, nil)
end

function remove_ap_group()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V16")
    if code  == 0 then
        code = networkImpl.remove_ap_group(arg_list_table)
    end
    sysutil.set_easy_return(code, nil)
end

function get_ap_list()
    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V16")
    if code  == 0 then
        code,result = networkImpl.get_ap_list(arg_list_table)
    end
    sysutil.set_easy_return(code, result)
end

function set_ap()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V16")
    if code  == 0 then
        code = networkImpl.set_ap(arg_list_table)
    end
    sysutil.set_easy_return(code, nil)
end

function delete_ap()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V16")
    if code  == 0 then
        code = networkImpl.delete_ap(arg_list_table)
    end
    sysutil.set_easy_return(code, nil)
end

function pctl_url_check()
    local result = {}
    local code = 0
    code,result = networkImpl.pctl_url_check(arg_list_table)
    sysutil.set_easy_return(code, result)
end

function get_freq_intergration()
    local result = {}
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V17")
    if code  == 0 then
        code,result = wirelessImpl.get_freq_intergration()
    end
    sysutil.set_easy_return(code, result)
end

function set_freq_intergration()
    local arg_list_table = get_arg_list()
    local code = protocol_check(arg_list_table["version"], "V17")
    if code  == 0 then
        code = wirelessImpl.set_freq_intergration(arg_list_table)
    end
    sysutil.set_easy_return(code, nil)
end

function set_samba()
    local arg_list_table = get_arg_list()
	local code = protocol_check(arg_list_table["version"], "V18")
	if code  == 0 then
		code = networkImpl.set_samba(arg_list_table)
	end
    sysutil.set_easy_return(code, nil)
end

function get_samba()
    local result = {}
    local arg_list_table = get_arg_list()
	local code = protocol_check(arg_list_table["version"], "V18")
	if code  == 0 then
		code,result = networkImpl.get_samba()
	end
    sysutil.set_easy_return(code, result)
end

function get_arg_list()
    local arg_list, data_len = luci.http.content()
    local arg_list_table = json.decode(arg_list)
    return arg_list_table
end
function func_adapter()
	local result = {}
	local arg_list_table = get_arg_list()
	local code = protocol_check(arg_list_table["version"], "V18")
	if code  == 0 then
		code,result = networkImpl.func_adapter(arg_list_table)
	end
	sysutil.set_easy_return(code, result)
end

--TODO change the alarm info in aispeech sever, to avoid getting uncorrect result when search alarm
function get_alarm()
	local result = {}
	local cmd = [[crontab -l | sed "s/\*/R/g"]]
	local arg_list_table = get_arg_list()
	local code = protocol_check(arg_list_table["version"], "V18")
	if code  == 0 then
		if arg_list_table["type"] == 0 or arg_list_table["type"] == 1 then
			result["type"] = arg_list_table["type"]
			result["result"] = {}
			local type = "闹钟"
			local t = io.popen(cmd)
			local content = t:read("*all")
			local index=1
			if arg_list_table["type"] == 1 then
				type = "日程"
			end
			for line in string.gmatch(content, "[^%c]+%c") do
				local list={}
				local tmp={}
				if nil ~= string.find(line, string.format("sh %s", type)) then
					string.gsub(string.gsub(line, "\n", " "), "[^ ]+", function(w) table.insert(tmp, w) end)
					list["min"] = tmp[1]
					list["hour"] = tmp[2]
					--list["day"] = tmp[3]
					if tmp[3] ~= "R" then
						convert=""
						if string.find(tmp[3], "-") ~= nil then
							s,e=string.match(tmp[3], "(%d)%-(%d+)")
							s=tonumber(s)
							e=tonumber(e)
							while (s <= e) do
								convert = string.format("%s,%d", convert, s)
								s=s+1
							end
						else
							convert=string.format(",%s", tmp[3])
						end
						list["day"] = string.gsub(convert, ",", "D")
					else
						if tmp[5] ~= "R" then
							convert=""
							if string.find(tmp[5], "-") ~= nil then
								s,e=string.match(tmp[5], "(%d)%-(%d)")
								s=tonumber(s)
								e=tonumber(e)
								if e == 0 then
									e=7
								end
								while (s <= e) do
									convert = string.format("%s,%d", convert, s)
									s=s+1
								end
							else
								convert=string.format(",%s", tmp[5])
							end
							list["dow"] = string.gsub(convert, ",", "W")
						end
					end
					list["mon"] = tmp[4]
					--list["dow"] = tmp[5]
					list["type"] = tmp[7]
					list["notify"] = tmp[8]
					list["sec"] = tmp[9]
					list["year"] = tmp[10]
					list["vid"] = tmp[11]
					result["result"][index] = list
					index=index+1
				end
			end 
		end
	end
	sysutil.set_easy_return(code, result)
end

function del_alarm()
	local result = {}
	local arg_list_table = get_arg_list()
	local code = protocol_check(arg_list_table["version"], "V18")
	if code  == 0 then
		if nil ~= arg_list_table["contents"] and arg_list_table["type"] ~= nil then
			local tmp = {}
			tmp["intent"] = "删除提醒"
			tmp["slots"] = {}
			local vids=""
			for i=1, #arg_list_table["contents"] do
				vid=arg_list_table["contents"][i]["vid"]
				if vid ~= nil then
					if vids == "" then
						vids=vid
					else
						vids=string.format("%s,%s", vids, vid)
					end
				else
					vids=""
				end
			end
			if vids == "" then
				code = 1
			else
				tmp["slots"]["vid"] = vids
				if arg_list_table["type"] == 1 then
					tmp["slots"]["对象"] = "日程"
				else
					tmp["slots"]["对象"] = "闹钟"
				end
				tmp["slots"]["操作"] = "删除"
				tmp["slots"]["ignore_tts"] = "yes"
				local cmd = string.format([[ubus call sf_mm_ubus event_notify '{"event_id" : 10, "data1" : "%s"}']], string.gsub(json.encode(tmp), [["]], [[\"]]))
				local t = io.popen(cmd)
				local content = t:read("*all")
				local tb = json.decode(content)
				if tb["code"] ~= nil then
					code = tb["code"]
				else
					code = 1
				end
			end
		else
			code=1
		end
	end
	sysutil.set_easy_return(code, result)
end

function add_alarm()
	local result = {}
	local arg_list_table = get_arg_list()
	local code = protocol_check(arg_list_table["version"], "V18")
	if code  == 0 then
		if arg_list_table["type"] ~= nil and arg_list_table["hour"] ~= nil and arg_list_table["min"] ~= nil and arg_list_table["sec"]~= nil then
			local tmp = {}
			tmp["intent"] = "创建提醒"
			tmp["slots"]={}
			if arg_list_table["type"] == 1 then
				tmp["slots"]["对象"] = "日程"
				if arg_list_table["notify"] == nil then
					code=1
				end
				tmp["slots"]["事件"] = arg_list_table["notify"]
			else
				tmp["slots"]["对象"] = "闹钟"
			end
			if tonumber(arg_list_table["hour"]) <= 12 then
				tmp["slots"]["时间段"] = "上午"
			else
				tmp["slots"]["时间段"] = "下午"
			end
			if arg_list_table["hour"] == nil or arg_list_table["min"] == nil or arg_list_table["sec"] == nil then
				code = 1
			else
				tmp["slots"]["时间"] = string.format("%s:%s:%s", arg_list_table["hour"], arg_list_table["min"], arg_list_table["sec"])
			end

			if arg_list_table["day"] ~= nil then
				if arg_list_table["day"] ~= "R" then
					if arg_list_table["year"] == nil or arg_list_table["mon"] == nil or arg_list_table["day"] == nil then
						code = 1
					else
						tmp["slots"]["日期"] = string.format("%s%s%s", arg_list_table["year"], arg_list_table["mon"], arg_list_table["day"])
					end
				else
					tmp["slots"]["repeat"] = "EVERYDAY" --[repeat everyday]--
				end
			else
				if arg_list_table["dow"] ~= nil then
					tmp["slots"]["repeat"] = string.gsub(arg_list_table["dow"], "W0", "W7")
				else
					code=1
				end
			end
			tmp["slots"]["操作"] = "创建"
			tmp["slots"]["ignore_tts"] = "yes"
			if code == 0 then
				local cmd = string.format([[ubus call sf_mm_ubus event_notify '{"event_id" : 10, "data1" : "%s"}']], string.gsub(json.encode(tmp), [["]], [[\"]]))
				local t = io.popen(cmd)
				local content = t:read("*all")
				local tb = json.decode(content)
				if tb["code"] ~= nil then
					code = tb["code"]
				else
					code = 1
				end
			end
		else
			code=1
		end
	end
	sysutil.set_easy_return(code, result)
end


function get_volume()
	local result = {}
	local arg_list_table = get_arg_list()
	local code = protocol_check(arg_list_table["version"], "V18")
	if code  == 0 then
		local t = io.popen([[ubus call sf_mm_ubus event_notify "{\"event_id\": 4}"]])
		local content = t:read("*all")
		result = json.decode(content)
	end
	sysutil.set_easy_return(code, result)
end

function set_volume()
	local result = {}
	local arg_list_table = get_arg_list()
	local code = protocol_check(arg_list_table["version"], "V18")
	if code  == 0 then
		if nil ~= arg_list_table["volume"] then
			local volume = arg_list_table["volume"]
			local cmd = string.format([[ubus call sf_mm_ubus event_notify '{"event_id" : 5, "data1" : "%d"}']], volume);
			local t = io.popen(cmd)
		end
	end
	sysutil.set_easy_return(code, result)
end

function get_sleep()
	local result = {}
	local arg_list_table = get_arg_list()
	local code = protocol_check(arg_list_table["version"], "V18")
	if code  == 0 then
		local cmd = [[ps | grep "sf-mm" | grep -v "grep"]]
		local t = io.popen(cmd)
		local content = t:read("*all")
		local sleeping = 0
		if content == "" then
			sleeping =  1
		else
			sleeping =  0
		end
		result["sleeping"] = sleeping
	end
	sysutil.set_easy_return(code, result)
end

function set_sleep()
	local result = {}
	local arg_list_table = get_arg_list()
	local code = protocol_check(arg_list_table["version"], "V18")
	local cmd = ""
	if code  == 0 then
		if arg_list_table["sleeping"] == 1 then
			cmd = [[/etc/init.d/sfmmd stop]]
		elseif arg_list_table["sleeping"] == 0 then
			cmd = [[/etc/init.d/sfmmd restart silence_boot]]
		end
		if cmd ~= "" then
			local t = io.popen(cmd)
		end
	end
	sysutil.set_easy_return(code, result)
end

function get_music_info()
	local result = {}
	local arg_list_table = get_arg_list()
	local code = protocol_check(arg_list_table["version"], "V18")
	if code  == 0 then
		local cmd = ""
		if arg_list_table["cmd"] == nil or arg_list_table["cmd"] == 0 then
			cmd = [[ubus call sf_mm_ubus event_notify "{\"event_id\":6}"]]
		else
			cmd = [[ubus call sf_mm_ubus event_notify "{\"event_id\":6, \"data1\" : \"1\"}"]]
		end
		local t = io.popen(cmd)
		local content = t:read("*all")
		local tb = json.decode(content)
		if nil ~= tb then
			result["contents"] = {}
			for i= 1, #(tb["contents"]) do
				table.insert(result["contents"], json.decode(tb["contents"][i]))
			end
		else
			code = 1
		end
	end
	sysutil.set_easy_return(code, result)
end


function media_control()
	local result = {}
	local arg_list_table = get_arg_list()
	local code = protocol_check(arg_list_table["version"], "V18")
	if code  == 0 then
		if arg_list_table["command"] ~= "" then
			local cmd = string.format([[ubus call sf_mm_ubus event_notify '{"event_id" : 7, "data1" : "%d"}']], arg_list_table["command"])
			local t = io.popen(cmd)
		end
	end
	sysutil.set_easy_return(code, result)
end

local function read_collection_from_file()
	local collections = {}
	local file = io.open("/usr/share/sf-mm/collections", "a+")	
	collection = file:read("*a")
	file:close()
	return json.decode(collection)
end

local function write_collection_to_file(collection)
	local file = io.open("/usr/share/sf-mm/collections", "w+")	
	file:write(json.encode(collection))
	file:close()
end

function get_media_states()
	local result = {}
	local arg_list_table = get_arg_list()
	local code = protocol_check(arg_list_table["version"], "V18")
	if code  == 0 then
		local cmd = [[ubus call sf_mm_ubus event_notify "{\"event_id\":8}"]]
		local t = io.popen(cmd)
		local content = t:read("*all")
		local tb = json.decode(content)
		if tb ~= nil then
			local collection = read_collection_from_file()
			local info = json.decode(tb["info"])
			if info ~= nil then
				local url = info["linkUrl"]
				if url ~= nil then                                         
					if collection == nil or collection[url] == nil then
						tb["collected"]=0
					else
						tb["collected"]=1
					end
					tb["info"]=info
					result=tb
				end
			end
		end
	end
	sysutil.set_easy_return(code, result)
end

function add_to_collection()
	local result = {}
	local arg_list_table = get_arg_list()
	local code = protocol_check(arg_list_table["version"], "V18")
	if code  == 0 then
		local music = arg_list_table["music"]
		if nil ~= music then
			local url = music["linkUrl"]
			if nil ~= url and "" ~= url then
				collection = read_collection_from_file()
				if collection == nil then
					collection = {}
				end
				collection[url] = arg_list_table["music"]
				write_collection_to_file(collection)
			end
		end
	end
	sysutil.set_easy_return(code, result)
end

function get_collection()
	local result = {}
	local arg_list_table = get_arg_list()
	local code = protocol_check(arg_list_table["version"], "V18")
	if code  == 0 then
		result["contents"] = {}
		local tmp = read_collection_from_file()
		if tmp ~= nil then
			for key, value in pairs(tmp)
			do
					table.insert(result["contents"], value)
			end
		end
	end
	sysutil.set_easy_return(code, result)
end

local function table_leng(t)
	local leng=0
	for k, v in pairs(t) do
		leng=leng+1
	end
	return leng
end

function cancel_collection()
	local result = {}
	local arg_list_table = get_arg_list()
	local code = protocol_check(arg_list_table["version"], "V18")
	if code  == 0 then
		local url = arg_list_table["url"]
		if nil ~= url and "" ~= url then
			local collection = read_collection_from_file()
			if collection ~= nil then
				if table_leng(collection) ~= 0 then
					collection[url] = nil
				else
					collection = nil
				end
				write_collection_to_file(collection)
			end
		end
	end
	sysutil.set_easy_return(code, result)
end

function media_play()
	local result = {}
	local arg_list_table = get_arg_list()
	local code = protocol_check(arg_list_table["version"], "V18")
	if code  == 0 then
		local music = arg_list_table["music"]
		if nil ~= music then
			local cmd = string.format([[ubus call sf_mm_ubus event_notify "{'event_id' : 9, 'data1' : '%s'}"]], string.gsub(json.encode(music), [["]], [[\"]]))
			local t = io.popen(cmd)
		else
			code = 1
		end
	end
	sysutil.set_easy_return(code, result)
end

function get_security_mode()
	local result = {}
	local arg_list_table = get_arg_list()
	local code = protocol_check(arg_list_table["version"], "V18")
	if code  == 0 then
		result["security_mode"] = deviceImpl.get_security_mode()
	end
	sysutil.set_easy_return(code, result)
end

function set_security_mode()
	local result = {}
	local arg_list_table = get_arg_list()
	local code = protocol_check(arg_list_table["version"], "V18")
	if code  == 0 then
		local enable = 0
		if arg_list_table["enable"] ~= 0 then
			enable = 1
		end
		local pid = nixio.fork()
		if pid == 0 then
			os.execute("sleep 1") 
			deviceImpl.set_security_mode(enable)
		end
	end
	if pid ~= 0 then
		sysutil.set_easy_return(code, result)
	end
end
--[[
function get_green_mode_enable()
	local result = {}
	local arg_list_table = get_arg_list()
	local code = protocol_check(arg_list_table["version"], "V18")
	if code  == 0 then
		result["enable"] = deviceImpl.get_green_mode_enable()
	end
	sysutil.set_easy_return(code, result)
end

function set_green_mode_enable()
	local result = {}
	local arg_list_table = get_arg_list()
	local code = protocol_check(arg_list_table["version"], "V18")
	if code  == 0 then
		if arg_list_table["enable"] ~= nil then
			code = deviceImpl.set_green_mode_enable(arg_list_table["enable"])
		else
			code = 1
		end
	end
	sysutil.set_easy_return(code, result)
end

function get_green_mode_config()
	local result = {}
	local arg_list_table = get_arg_list()
	local code = protocol_check(arg_list_table["version"], "V18")
	if code  == 0 then
		local limit_info = deviceImpl.get_green_mode_config()
		result["start_time"] = limit_info["start_time"]
		result["end_time"] = limit_info["end_time"]
		result["limit"] = limit_info["limit"]
	end

	sysutil.set_easy_return(code, result)
end

function set_green_mode_config()
	local result = {}
	local arg_list_table = get_arg_list()
	local code = protocol_check(arg_list_table["version"], "V18")
	if code  == 0 then
		local start_time = arg_list_table["start_time"]
		local end_time = arg_list_table["end_time"]
		local limit  = arg_list_table["limit"]
		if start_time ~= nil and end_time ~= nil and limit ~= nil then
			deviceImpl.set_green_mode_config(start_time, end_time, limit)
		else
			code=1
		end
	end
	sysutil.set_easy_return(code, result)
end

function get_green_mode_device()
	local result = {}
	local arg_list_table = get_arg_list()
	local code = protocol_check(arg_list_table["version"], "V18")
	if code  == 0 then
		result["device"] = deviceImpl.get_green_mode_device()
	end
	sysutil.set_easy_return(code, result)
end

function remove_green_mode_device()
	local result = {}
	local arg_list_table = get_arg_list()
	local code = protocol_check(arg_list_table["version"], "V18")
	if code  == 0 then
		if arg_list_table["device"] ~= nil then
			code = deviceImpl.remove_green_mode_device(arg_list_table["device"])
		else
			code = 1
		end
	end
	sysutil.set_easy_return(code, result)
end
]]--

function get_green()
	local result = {}
	local arg_list_table = get_arg_list()
	local code = protocol_check(arg_list_table["version"], "V18")
	local device = {}
	local tmp = {}
	if code  == 0 then
		result["enable"] = deviceImpl.get_green_mode_enable()
		local limit_info = deviceImpl.get_green_mode_config()
		result["start_time"] = limit_info["start_time"]
		result["end_time"] = limit_info["end_time"]
		result["limit"] = limit_info["limit"]
		result["device"] = {}
		tmp = deviceImpl.get_green_mode_device()
		for key, value in pairs(tmp) do
			device["mac"] = key
			device["name"] = value
			table.insert(result["device"], device)
		end
	end
	sysutil.set_easy_return(code, result)
end


function set_green()
	local result = {}
	local arg_list_table = get_arg_list()
	local code = protocol_check(arg_list_table["version"], "V18")
	local tmp = {}
	local device = {}
	if code  == 0 then
		if arg_list_table["enable"] ~= nil then
			code = deviceImpl.set_green_mode_enable(arg_list_table["enable"])
		elseif arg_list_table["device"] ~= nil then
			tmp = arg_list_table["device"]
			for i=1, #tmp do
				mac=tmp[i].mac
				name=tmp[i].name
				device[mac] = name
			end
			code = deviceImpl.remove_green_mode_device(device)
		elseif arg_list_table["start_time"] ~= nil and arg_list_table["end_time"] ~= nil and arg_list_table["limit"] ~= nil then
			code = deviceImpl.set_green_mode_config(arg_list_table["start_time"], arg_list_table["end_time"], arg_list_table["limit"])
		else
			code = 1
		end
	end
	sysutil.set_easy_return(code, result)
end
