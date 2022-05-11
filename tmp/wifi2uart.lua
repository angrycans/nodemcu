print("hello nodemcu => wifi2lua")
---------------------------------------------
baud = 9600
serverPort = 9100
quitProxyCmd = "--quitproxy"
---------------------------------------------
client = nil

function receiver(sck, data)
 -- print("receive data=>"..data)
  client:send("receive ok")
  uart.write(0, 0x1b,0x40,0x31,0x31,0x31,0x0a)
  --uart.write(0, data)
 -- sck:close()
end

function netdisconnect(c)
  print("net disconnect")
  --client = nil
end

sv = net.createServer(net.TCP, 2000)
if sv then
  sv:listen(serverPort, function(conn)
    client=conn
    client:on("receive", receiver)
    
    client:send("receive ok")

    client:on('disconnection',netdisconnect)
  end)
end

function  uartOndata(data)
    if (client~=nil) then
        --printf("uartOndata=>"..data)
        --client:send(data)
    end
end

uart.setup(0, baud, 8, 0, 1, 0)

uart.on("data", uartOndata)

print("\nScript by angrycans@gmail.com\nSerialNet proxy running on " .. wifi.sta.getip()..":".. serverPort)
