print("start  free mem:", node.heap())
wifi.setmode(wifi.SOFTAP)
local apcfg = {ssid = "RaceLap", pwd = "88888888"}
wifi.ap.config(apcfg)

require("httpserver"):listen(80)

print("end  free mem:", node.heap())
