--siwifi solution by tommy.qiu@siflower.com.cn

m = Map("wireless", translate("Wireless Network"))

s = m:section(TypedSection, "wifi-iface", nil)
s:depends("ifname", "wlan0")
s.anonymous = true

s2 = m:section(TypedSection, "wifi-iface", nil)
s2:depends("ifname", "wlan1")
s2.anonymous = true

band = s:option(DummyValue, "band", "2.4G")
ssid = s:option(Value, "ssid", "SSID")
ssid.datatype = "maxlength(32)"
pw = s:option(Value, "key", translate("Password"))
pw.password = true
pw.datatype = "wpakey"
pw.rmempty = true

band2 = s2:option(DummyValue, "band", "5G")
ssid2 = s2:option(Value, "ssid", "SSID")
ssid2.datatype = "maxlength(32)"
pw2 = s2:option(Value, "key", translate("Password"))
pw2.password = true
pw2.datatype = "wpakey"
pw2.rmempty = true

function m.on_commit(map)
	m.message = translate("Password successfully changed!")
end

op = s2:option(Button, "_btn", translate("Detailed settings"), "")
op.template = "admin_quicksettings/wifi"

return m

