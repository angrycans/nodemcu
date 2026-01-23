import serial
import time
import struct

# ===== 串口配置 =====
# PORT = "COM5"          # Windows
# PORT = "/dev/ttyUSB0"  # Linux
# 
PORT = "/dev/cu.usbmodem5B3E1427651"  # macOS

BAUD = 115200
TIMEOUT = 1


def send_flypt_Y4(ser, axis_values):
    """
    axis_values: list of uint16, e.g. [32768, 0, 0]
    """
    if len(axis_values) != 3:
        raise ValueError("Need exactly 3 axis values")

    frame = bytearray()

    # 帧头
    frame += b'Y'
    frame += b'4'

    # 3 轴，每轴 16bit 大端
    for v in axis_values:
        v = max(0, min(65535, int(v)))
        frame += struct.pack('>H', v)

    # 补 3 轴 0（你示例里的 000000）
    for _ in range(3):
        frame += struct.pack('>H', 0)

    # 帧尾
    frame += b'!!'

    print("TX:", frame)
    ser.write(frame)


def main():
    with serial.Serial(PORT, BAUD, timeout=TIMEOUT) as ser:
        print("Serial opened:", ser.name)
        time.sleep(2)  # 等 ESP32 重启

      
        # ===== 示例 2：1 号轴 → 50mm 对应值 =====
        # 如果 300mm 行程：50 / 300 * 65535
        axis1_50mm = int(100 / 100 * 65535)
        axis2_40mm = int(90 / 100 * 65535)

        send_flypt_Y4(ser, [axis1_50mm, axis2_40mm, 0])

        time.sleep(1)

        # ===== 示例 3：1 号轴回到 0 =====
        send_flypt_Y4(ser, [0, 0, 0])

        print("Test done.")


if __name__ == "__main__":
    main()
