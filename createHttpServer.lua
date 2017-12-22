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
        print("1\n")
        uart.setup(0, 9600, 8, 0, 1, 0)

        uart.write(0, string.char(0x1b, 0x40, 0x12, 0x54))

        --
        --uart.setup(1, 9600, 8, 0, 1, 0)
        --uart.write(1, string.char(0x1b, 0x40, 0x12, 0x54))
        --print(string.char(0x1b, 0x40, 0x12, 0x54))

        --uart.write(0, 0x1B, 0x40, 0x12, 0x54)

        --res:type("application/json")
        res:send("send ok")
    end
)

httpServer:use(
    "/printer2",
    function(req, res)
        --print("get /printer")
        --print("\x1b\x40")
        print("2\n")
        uart.setup(0, 9600, 8, 0, 1, 0)
        --print(string.char(0x1b, 0x40, 0x12, 0x54))

        --print("body=>" .. req.body)

        if (req.method == "POST") then
            string.gsub(
                req.body,
                "[^,]+",
                function(n)
                    --print("data=>" .. n)
                    uart.write(0, string.char(n))
                end
            )
        --uart.write(0, string.char(27, 64, 18, 94))
        --uart.write(0, string.char(0x1b, 0x40, 0x12, 0x54))
        end

        res:send("send ok")
    end
)

httpServer:use(
    "/printer3",
    function(req, res)
        --print("get /printer")
        --print("\x1b\x40")
        print("3\n")
        uart.setup(0, 9600, 8, 0, 1, 0)
        uart.write(0, string.char(27, 64, 18, 94))

        res:send("send ok")
    end
)
httpServer:use(
    "/printer4",
    function(req, res)
        --print("get /printer")
        --print("\x1b\x40")
        print("4\n")
        uart.setup(0, 9600, 8, 0, 1, 0)
        uart.write(0, string.char(0x1b))
        uart.write(0, string.char(0x40))
        uart.write(0, string.char(0x12))
        uart.write(0, string.char(0x54))
        res:send("send ok")
    end
)
httpServer:use(
    "/printer5",
    function(req, res)
        --print("get /printer")
        --print("\x1b\x40")
        print("5\n")
        uart.setup(0, 9600, 8, 0, 1, 0)
        uart.write(0, 0x1B, 0x40, 0x12, 0x54)
        res:send("send ok")
    end
)

httpServer:use(
    "/printer6",
    function(req, res)
        print("6\n")
        uart.setup(0, 9600, 8, 0, 1, 0)
        if (req.method == "POST") then
            string.gsub(
                req.body,
                "[^,]+",
                function(n)
                    print("n=" .. n .. "\n")
                    uart.write(0, string.char(tonumber(n, 16)))
                end
            )
        end
        res:send("send ok")
    end
)
