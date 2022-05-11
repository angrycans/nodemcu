--Reset button
GPIO_RESET = 1

reset_count = 0
gpio.mode(GPIO_RESET, gpio.INPUT, gpio.PULLUP)
function Pin_ResetStatus()
    local key = gpio.read(1)
    print("GPIO_RESET=>" .. key .. "\n")
    if (key == 0) then
        reset_count = reset_count + 1
    else
        if (reset_count > 6) then
            print("GPIO_RESET=>push down \n")
            node.restart()
        end
        reset_count = 0
    end
end

reset_timer = tmr.create()
tmr.register(reset_timer, 500, tmr.ALARM_AUTO, Pin_ResetStatus)
tmr.start(reset_timer)
