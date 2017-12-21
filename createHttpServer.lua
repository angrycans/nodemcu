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

-- uart.on(
--     "data",
--     function(data)
--         print("uartOndata=>" .. data)
--     end
-- )
httpServer:use(
    "/printer",
    function(req, res)
        --print("get /printer")
        --print("\x1b\x40")

        --print(string.char(0x1b, 0x40, 0x12, 0x54))

        print("get /printer")

        --uart.write(0, 0x00, 0x1B, 0x40)

        uart.setup(0, 9600, 8, 0, 1, 0)
        uart.write(0, string.char(0x1b, 0x40, 0x12, 0x54))
        --uart.setup(1, 9600, 8, 0, 1, 0)
        --uart.write(1, string.char(0x1b, 0x40, 0x12, 0x54))
        --print(string.char(0x1b, 0x40, 0x12, 0x54))

        --uart.write(0, 0x1B, 0x40, 0x12, 0x54)

        --res:type("application/json")
        res:send("send ok")
    end
)
