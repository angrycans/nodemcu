print(gpio.read(0), "\n")
gpio.mode(0, gpio.OUTPUT)
print(gpio.read(0), "\n")
gpio.write(0, gpio.LOW)
print(gpio.read(0), "\n")
gpio.write(0, gpio.HIGH)
print(gpio.read(0), "\n")

