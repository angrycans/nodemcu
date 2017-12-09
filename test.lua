print("hello nodemcu => connect wifi_cola")

print(wifi.sta.getip())

-- Print AP list that is easier to read
function listap(t) -- (SSID : Authmode, RSSI, BSSID, Channel)
    print("\n"..string.format("%32s","SSID").."\tBSSID\t\t\t\t  RSSI\t\tAUTHMODE\tCHANNEL")
    for ssid,v in pairs(t) do
        local authmode, rssi, bssid, channel = string.match(v, "([^,]+),([^,]+),([^,]+),([^,]+)")
        print(string.format("%32s",ssid).."\t"..bssid.."\t  "..rssi.."\t\t"..authmode.."\t\t\t"..channel)
    end
end
wifi.sta.getap(listap)

function connected_cb(ssid,bssid,channel)
print("callback connected_cb")
end
function disconnected_cb(ssid,bssid,reason)
print("callback disconnected_cb")
end
wifi.eventmon.register(wifi.eventmon.STA_CONNECTED, function(T)
 print("\n\tSTA - CONNECTED".."\n\tSSID: "..T.SSID.."\n\tBSSID: "..
 T.BSSID.."\n\tChannel: "..T.channel)
 end)

wifi.eventmon.register(wifi.eventmon.STA_DISCONNECTED, function(T)
 print("\n\tSTA - DISCONNECTED".."\n\tSSID: "..T.SSID.."\n\tBSSID: "..
 T.BSSID.."\n\treason: "..T.reason)
 end)

  wifi.eventmon.register(wifi.eventmon.STA_GOT_IP, function(T)
 print("\n\tSTA - GOT IP".."\n\tStation IP: "..T.IP.."\n\tSubnet mask: "..
 T.netmask.."\n\tGateway IP: "..T.gateway)
 end)
wifi.setmode(wifi.STATION)
station_cfg={}
station_cfg.ssid="wifi_cola"
station_cfg.pwd="85750218"
station_cfg.connected_cb=connected_cb
station_cfg.disconnected_cb=disconnected_cb
station_cfg.save=true
station_cfg.auto=false
wifi.sta.config(station_cfg)


wifi.sta.connect()
