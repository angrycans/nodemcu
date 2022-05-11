--BLINK
IO_BLINK = 4
TMR_BLINK = 5

blink = nil
gpio.mode(IO_BLINK, gpio.OUTPUT)

tmr.register(
    TMR_BLINK,
    100,
    tmr.ALARM_AUTO,
    function()
        gpio.write(IO_BLINK, blink.i % 2)
        tmr.interval(TMR_BLINK, blink[blink.i + 1])
        blink.i = (blink.i + 1) % #blink
    end
)

function blinking(param)
    if type(param) == "table" then
        blink = param
        blink.i = 0
        tmr.interval(TMR_BLINK, 1)
        running, _ = tmr.state(TMR_BLINK)
        if running ~= true then
            tmr.start(TMR_BLINK)
        end
    else
        tmr.stop(TMR_BLINK)
        gpio.write(IO_BLINK, param or gpio.LOW)
    end
end

blinking({200, 200})
