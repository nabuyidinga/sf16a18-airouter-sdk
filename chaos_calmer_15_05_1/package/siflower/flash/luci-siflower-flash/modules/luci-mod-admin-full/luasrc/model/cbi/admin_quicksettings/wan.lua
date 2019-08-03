--siflower solution by tommy.qiu@siflower.com.cn

m = Map("network", translate("Internet Connection"))
s = m:section(NamedSection, "wan", "interface", nil)

p = s:option(ListValue, "proto", translate("Protocol"))
p:value("static", translate("Static address"))
p:value("dhcp", translate("DHCP client"))
p:value("pppoe", translate("PPPoE"))

ipaddr = s:option(Value, "ipaddr", translate("IP address"))
ipaddr:depends("proto","static")
ipaddr.datatype = "host"
ipaddr.rmempty = true

netmask = s:option(Value, "netmask", translate("Netmask"))
netmask :depends("proto", "static")
netmask.datatype = "host"
netmask.rmempty = true

gateway = s:option(Value, "gateway", translate("Gateway"))
gateway:depends("proto", "static")
gateway.datatype = "host"
gateway.rmempty = true

username = s:option(Value, "username", translate("Username"))
username:depends("proto", "pppoe")
username.datatype = "maxlength(64)"
username.rmempty = true

password = s:option(Value, "password", translate("Password"))
password:depends("proto", "pppoe")
password.password = true
password.rmempty = true

dns = s:option(Value, "dns",translate("DNS"))
dns.datatype = "host"
dns.rmempty = true

function m:on_commit(map)
	m.message = translate("Network successfully changed!")
end

op = s:option(Button, "_btn", translate("Detailed settings"), "")
op.template = "admin_quicksettings/wan"

return m

