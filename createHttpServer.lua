print("createHttpserver start...")

dofile("httpServer.lua")
httpServer:listen(80)

--custome require

httpServer:use(
    "/scanap",
    function(req, res)
        wifi.sta.getap(
            function(t)
                local aptable = {}
                for ssid, v in pairs(t) do
                    local authmode, rssi, bssid, channel = string.match(v, "([^,]+),([^,]+),([^,]+),([^,]+)")
                    aptable[ssid] = {
                        authmode = authmode,
                        rssi = rssi,
                        bssid = bssid,
                        channel = channel
                    }
                end
                res:type("application/json")
                res:send(sjson.encode(aptable))
            end
        )
    end
)

httpServer:use(
    "/getsysstatus",
    function(req, res)
        res:type("application/json")
        res:send(sjson.encode(getSysStatus()))
    end
)
