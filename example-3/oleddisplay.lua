-- 管脚定义
local sda = 3 -- --SDA-->D3
local scl = 4 -- --SCL-->D4
local sla = 0x3c -- oled的地址，一般为0x3c
-- 初始化
function init_oled()
  -- iic总线 和 oled初始化
  i2c.setup(0, sda, scl, i2c.SLOW)
  disp = u8g2.ssd1306_i2c_128x64_noname(0, sla)
  -- 设置字体
  disp:setFont(u8g2.font_unifont_t_symbols)
  disp:setFontRefHeightExtendedText()
  --disp:setDrawColor(1)
  disp:setFontPosTop()
  --disp:setFontDirection(0)
  -- 画边框
  --disp:drawFrame(0, 0, 128, 64)
end
-- 显示函数
function oled_show_msg()
  -- 设置显示内容
  disp:drawStr(0, 0, "Hello OLEDISPLAY")
  disp:drawStr(0, 16, "postion")
  disp:drawStr(0, 32, "nihao")
  disp:drawStr(0, 48, "ABCDEFGHIJKLMNOPQRSTUVW")
  -- 将内容发送到oled
  disp:sendBuffer()
end
-- 主函数
function main()
  init_oled()
  oled_show_msg()
end
-- 运行程序
main()
