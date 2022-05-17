print("hello nodemcu => softuart")
---------------------------------------------

local TX_PIN = 2
local RX_PIN = 3
local BAUD_RATE = 9600
local STR_TERMINATOR = "$"
local s = softuart.setup(BAUD_RATE, TX_PIN, RX_PIN)

print(s)

s:on(
  "data",
  10,
  function(data)
    print("Lua handler called!")
    print(data)
  end
)

s:write("hello world")

--suart:on("data", STR_TERMINATOR, Ondata)

-- serverPort = 9100
-- quitProxyCmd = "--quitproxy"
---------------------------------------------
-- client = nil

-- function receiver(sck, data)
--  -- print("receive data=>"..data)
--   client:send("receive ok")
--   uart.write(0, 0x1b,0x40,0x31,0x31,0x31,0x0a)
--   --uart.write(0, data)
--  -- sck:close()
-- end

-- function uartOndata(data)
--   -- if (client~=nil) then
--   --     --printf("uartOndata=>"..data)
--   --     --client:send(data)
--   -- end

--   printf("uartOndata=>" .. data)
-- end

-- uart.setup(0, baud, 8, 0, 1, 0)

-- uart.on("data", uartOndata)
