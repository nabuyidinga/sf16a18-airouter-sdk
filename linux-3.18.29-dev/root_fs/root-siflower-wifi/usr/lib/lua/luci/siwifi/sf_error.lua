--[[
LuCI - Lua Configuration Interface

Description:
Offers an util to define siwifi errors
]]--


local disp = require "luci.dispatcher"

module ("luci.siwifi.sf_error", package.seeall)

--[[
  define error NO and message for siwifi return value
  error NO should between 1000 and 2000
  if we want to support multi language we should edit language files for /usr/lib/lua/luci/i18n
--]]

--operation not permit
ERROR_NO_OPERATION_NOT_PERMIT = 1001
--protocol version not found
ERROR_NO_PROTOCOL_NOT_FOUND = 1002
--protocol version not support
ERROR_NO_PROTOCOL_NOT_SUPPORT = 1003
--command not support
ERROR_NO_UNKNOWN_CMD = 1004
--old_password is incorrect while setting password
ERROR_NO_OLDPASSWORD_INCORRECT = 1005
--signal mode not support
ERROR_NO_UNKNOWN_SIGNAL_MODE = 1006
--no ssid exist in router
ERROR_NO_SSID_NONEXIST = 1007
--ssid doesn't match
ERROR_NO_SSID_UNMATCH = 1008
--reset is running
ERROR_NO_WAITTING_RESET = 1009
--can't get lan speed
ERROR_NO_CANNOT_GET_LANSPEED = 1010
--authentication fail
ERROR_NO_SESSION_OUT_DATE = 1011
--router has been bind
ERROR_NO_ROUTER_HAS_BIND = 1012
--empty userid
ERROR_NO_USERID_EMPTY = 1013
--internel socket error
ERROR_NO_INTERNAL_SOCKET_FAIL = 1014
--bind router fail
ERROR_NO_BIND_FAIL = 1015
--router has not bind yet
ERROR_NO_ROUTER_HAS_NOT_BIND = 1016
--unbind router fail
ERROR_NO_UNBIND_FAIL = 1017
--unbind fail case caller is not binder
ERROR_NO_CALLER_NOT_BINDER = 1018

--set device fail mac-addr is empty
ERROR_NO_MAC_EMPTY = 1019
--wifi setting param is empty
ERROR_NO_WIFI_SETTING_EMPTY = 1020

--execute check wan type exception
ERROR_NO_WAN_TYPE_EXCEPTION = 1051
--prase wan type result fail
ERROR_NO_WAN_TYPE_PARSER_FAIL = 1052
--execute get wan type exception
ERROR_NO_WAN_PROTO_EXCEPTION =1053
--execute set wan unable to get type
ERROR_NO_WANSET_TYPE_NOT_FOUND =1054
--execute set wan get a unnormal type
ERROR_NO_WANSET_TYPE_EXCEPTION =1055
--execute set wan unable to get dns input
ERROR_NO_WANSET_DNS_NOT_FOUND =1056


--check manager param fail
ERROR_NO_CHECK_MANAGER_PARAMS_FAIL = 1400
--do manager operation fail
ERROR_NO_MANAGER_OP_FAIL = 1401
--otaversion information not downloaded
EEROR_NO_OTAVESION_NOT_DOWNLOADED = 1500
--ota bin not downloaded successfully
EEROR_NO_CHECKSUM_NOT_MATCHED = 1501
--localversion the same with otaversion
EEROR_NO_LOCALVERSION_EAQULE_OTAVERSION = 1502
--waiting for ota upgrade
ERROR_NO_WAITTING_OTA_UPGRADE = 1503
--upload log file to bmob failed
EEROR_NO_UPLOAD_FILE_FAILED = 1504
--upload log url info failed
EEROR_NO_UPLOAD_INFO_FAILED = 1505
--waiting for another log upload
EEROR_NO_WAITING_UPLOAD_LOG = 1506

---------------------------------------------------
------------Define Gateway Error Code--------------
------------------Begin with 1601------------------
--the rule parameter is nil
ERROR_NO_GW_NIL_PARAMETER = 1601
--the rule parameter is illegal
ERROR_NO_GW_RULE_PARAM_ILLEGAL = 1602
--the get_zigbee_rule deliver an unknown type
ERROR_NO_GW_ILLEGAL_TYPE = 1603
--the database parameter is illegal
ERROR_NO_GW_ILLEGAL_DB_PARAM = 1604
--the device does not exist in database
ERROR_NO_GW_DEVICE_NOT_EXIST = 1605
--the setting devices no one online
ERROR_NO_GW_DEVICE_NO_DEVICE_ONLINE = 1606
--create zigbee rule failed
ERROR_NO_GW_CREATE_RULE_FAILED = 1607


function _(str)
    return disp._(str)
end

function translate(str)
    return disp.translate(str)
end


function getErrorMessage(errorCode)
    local errorList = {}
    --1000~1100 to define system error
    errorList[0]    = _("OK")
    errorList[1001] = _("operation is not permit")
    errorList[1002] = _("param protocol version not found")
    errorList[1003] = _("protocol version not support")
    errorList[1004] = _("unknown command")
    errorList[1005] = _("old password is incorrect")
    errorList[1006] = _("unknown signal_mode")
    errorList[1007] = _("no ssid exist in router")
    errorList[1008] = _("ssid doesn't match")
    errorList[1009] = _("reset is running, can't reset in this interval")
    errorList[1010] = _("can't get lan speed")
    errorList[1011] = _("authentication fail")
    errorList[1012] = _("router has been bind")
    errorList[1013] = _("empty userid")
    errorList[1014] = _("internel socket error")
    errorList[1015] = _("bind router fail")
    errorList[1016] = _("router has not bind yet")
    errorList[1017] = _("unbind router fail")
    errorList[1018] = _("unbind fail case caller is not binder")
    errorList[1019] = _("set device fail mac-addr is empty")
    errorList[1020] = _("set wifi fail the setting param is empty")
    errorList[1051] = _("execute check wan type exception")
    errorList[1052] = _("prase wan type result fail")
    errorList[1053] = _("execute get wan type exception")
    errorList[1054] = _("execute set wan unable to get type")
    errorList[1055] = _("execute set wan get a unnormal type")
    errorList[1056] = _("execute set wan unable to get dns input")

    errorList[1400] = _("check manager param fail")
    ERROR_NO_MANAGER_OP_FAIL = 1401
    errorList[1401] = _("do manager operation fail")
    --1500~1600 to define network error
    errorList[1500] = _("otaversion information not downloaded")
    errorList[1501] = _("checksum not match the ota_checksum")
    errorList[1502] = _("localversion the same with otaversion")
    errorList[1503] = _("waiting for ota upgrade")
    errorList[1504] = _("upload log file to bmob failed")
    errorList[1505] = _("upload log url info failed")
    errorList[1506] = _("waiting for another log upload")

    --1601~1700 to define gateway error
    errorList[1601] = _("some mandatory parameter is nil")
    errorList[1602] = _("some parameters of zigbee rule is illegal")
    errorList[1603] = _("the request type is unknown")
    errorList[1604] = _("the database parameter is illegal")
    errorList[1605] = _("the device does not exist in database")
    errorList[1606] = _("the setting devices no one online")
    errorList[1607] = _("create zigbee rule failed")

    if (errorList[errorCode] == nil) then
        return translate(_("unknown error"))
    else
        return translate(errorList[errorCode])
    end
end


function errProtocolNotFound()
    local err = {}
    err["code"] = ERROR_NO_PROTOCOL_NOT_FOUND
    err["msg"] = getErrorMessage(ERROR_NO_PROTOCOL_NOT_FOUND)
    return err
end

function errProtocolNotSupport()
    local err = {}
    err["code"] = ERROR_NO_PROTOCOL_NOT_SUPPORT
    err["msg"] = getErrorMessage(ERROR_NO_PROTOCOL_NOT_SUPPORT)
    return err
end


