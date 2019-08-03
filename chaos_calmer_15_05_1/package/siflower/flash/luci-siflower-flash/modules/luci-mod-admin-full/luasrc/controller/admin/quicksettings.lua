--A quick settings page
--By Tommy.qiu@siflower.com.cn

module("luci.controller.admin.quicksettings", package.seeall)

function index()
	entry({"admin", "quicksettings"}, firstchild(), "SiWifi Quick Settings", 25).dependent=false
	entry({"admin", "quicksettings", "admin"}, cbi("admin_quicksettings/admin"),"SiWifi Admin Password Setting", 10)
	entry({"admin", "quicksettings", "wan"}, cbi("admin_quicksettings/wan"), "SiWifi Wan Setting", 20)
	entry({"admin", "quicksettings", "wifi"}, cbi("admin_quicksettings/wifi"), "SiWiFi Wifi Password Setting", 30)
end

