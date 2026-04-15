import argparse
import struct
import time

import serial
from serial.tools import list_ports


AXIS_NUM = 7
POS_MAX = 65535
DEFAULT_BAUD = 115200
DEFAULT_TIMEOUT = 1.0
DEFAULT_OPEN_DELAY = 2.0
DEFAULT_STROKE_MM = 100.0
DEFAULT_WRITE_TIMEOUT = 1.0


def build_parser():
    parser = argparse.ArgumentParser(
        description="FlyPT binary tester for the 7-axis controller."
    )
    parser.add_argument("--port", help="Serial port, for example COM5 or /dev/ttyUSB0.")
    parser.add_argument("--baud", type=int, default=DEFAULT_BAUD, help="Serial baud rate.")
    parser.add_argument(
        "--write-timeout",
        type=float,
        default=DEFAULT_WRITE_TIMEOUT,
        help="Serial write timeout in seconds.",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=DEFAULT_TIMEOUT,
        help="Serial read timeout in seconds.",
    )
    parser.add_argument(
        "--open-delay",
        type=float,
        default=DEFAULT_OPEN_DELAY,
        help="Delay after opening the serial port, in seconds.",
    )
    parser.add_argument(
        "--reset-on-open",
        action="store_true",
        help="Allow DTR/RTS changes on open. By default the script keeps them low to avoid resetting ESP32.",
    )
    parser.add_argument(
        "--list-ports",
        action="store_true",
        help="List available serial ports and exit.",
    )
    parser.add_argument(
        "--raw",
        action="store_true",
        help="Interpret axis inputs as raw uint16 values instead of millimeters.",
    )
    parser.add_argument(
        "--axes",
        nargs="*",
        type=float,
        help="Axis values in order A1..A7. In mm by default, or raw if --raw is set.",
    )
    parser.add_argument(
        "--strokes",
        nargs="*",
        type=float,
        help="Per-axis stroke in mm for A1..A7. Used only in mm mode. Default is 100 for all axes.",
    )
    parser.add_argument(
        "--repeat",
        type=int,
        default=1,
        help="How many times to send the frame.",
    )
    parser.add_argument(
        "--interval",
        type=float,
        default=0.1,
        help="Interval between repeated sends, in seconds.",
    )
    parser.add_argument(
        "--interactive",
        action="store_true",
        help="Keep the serial port open and send multiple commands interactively.",
    )
    parser.add_argument(
        "--hold-open",
        type=float,
        default=0.0,
        help="Keep the serial port open for N seconds after sending before exit.",
    )
    parser.add_argument(
        "--wait-exit",
        action="store_true",
        help="After sending, keep the serial port open until you press Enter.",
    )

    for axis in range(1, AXIS_NUM + 1):
        parser.add_argument(
            f"--a{axis}",
            type=float,
            help=f"Override axis {axis} value.",
        )

    return parser


def list_serial_ports():
    ports = list(list_ports.comports())
    if not ports:
        print("No serial ports found.")
        return

    print("Available serial ports:")
    for port in ports:
        extra = f" - {port.description}" if port.description else ""
        print(f"  {port.device}{extra}")


def normalize_axis_values(cli_values, per_axis_values):
    values = [0.0] * AXIS_NUM

    if cli_values:
        if len(cli_values) > AXIS_NUM:
            raise ValueError(f"--axes accepts at most {AXIS_NUM} values.")
        for index, value in enumerate(cli_values):
            values[index] = value

    for index, value in enumerate(per_axis_values):
        if value is not None:
            values[index] = value

    return values


def apply_axis_assignments(values, assignments):
    for item in assignments:
        text = item.strip().lower()
        if "=" not in text:
            raise ValueError(f"Unsupported argument: {item}")

        axis_name, raw_value = text.split("=", 1)
        if not axis_name.startswith("a"):
            raise ValueError(f"Unsupported argument: {item}")

        try:
            axis_index = int(axis_name[1:]) - 1
        except ValueError as exc:
            raise ValueError(f"Unsupported axis name: {axis_name}") from exc

        if axis_index < 0 or axis_index >= AXIS_NUM:
            raise ValueError(f"Axis index out of range in argument: {item}")

        try:
            values[axis_index] = float(raw_value)
        except ValueError as exc:
            raise ValueError(f"Invalid axis value in argument: {item}") from exc

    return values


def normalize_strokes(strokes):
    values = [DEFAULT_STROKE_MM] * AXIS_NUM
    if not strokes:
        return values
    if len(strokes) > AXIS_NUM:
        raise ValueError(f"--strokes accepts at most {AXIS_NUM} values.")

    for index, value in enumerate(strokes):
        if value <= 0:
            raise ValueError("Stroke must be greater than 0.")
        values[index] = value

    return values


def mm_to_raw(mm_value, stroke_mm):
    raw = round((float(mm_value) / float(stroke_mm)) * POS_MAX)
    return clamp_u16(raw)


def clamp_u16(value):
    return max(0, min(POS_MAX, int(round(value))))


def build_frame(raw_values):
    if len(raw_values) != AXIS_NUM:
        raise ValueError(f"Need exactly {AXIS_NUM} axis values.")

    frame = bytearray()
    frame += b"Y"
    frame += b"7"

    for value in raw_values:
        frame += struct.pack(">H", clamp_u16(value))

    frame += b"!!"
    return frame


def send_flypt_frame(ser, raw_values):
    frame = build_frame(raw_values)
    print("TX raw:", list(raw_values))
    print("TX bytes:", " ".join(f"{byte:03d}" for byte in frame))
    ser.write(frame)
    ser.flush()


def open_serial_port(args):
    if args.reset_on_open:
        return serial.Serial(
            port=args.port,
            baudrate=args.baud,
            timeout=args.timeout,
            write_timeout=args.write_timeout,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            xonxoff=False,
            rtscts=False,
            dsrdtr=False,
            exclusive=False,
        )

    ser = serial.Serial()
    ser.port = args.port
    ser.baudrate = args.baud
    ser.timeout = args.timeout
    ser.write_timeout = args.write_timeout
    ser.bytesize = serial.EIGHTBITS
    ser.parity = serial.PARITY_NONE
    ser.stopbits = serial.STOPBITS_ONE
    ser.xonxoff = False
    ser.rtscts = False
    ser.dsrdtr = False
    ser.exclusive = False
    ser.dtr = False
    ser.rts = False
    ser.open()
    try:
        ser.setDTR(False)
        ser.setRTS(False)
    except (OSError, serial.SerialException):
        pass
    return ser


def print_serial_config(ser):
    print(
        "Serial config:",
        f"baud={ser.baudrate}",
        f"bytesize={ser.bytesize}",
        f"parity={ser.parity}",
        f"stopbits={ser.stopbits}",
        f"timeout={ser.timeout}",
        f"write_timeout={ser.write_timeout}",
        f"xonxoff={ser.xonxoff}",
        f"rtscts={ser.rtscts}",
        f"dsrdtr={ser.dsrdtr}",
    )


def safe_close_serial(ser):
    if ser is None:
        return
    if not ser.is_open:
        return

    try:
        ser.flush()
    except (OSError, serial.SerialException):
        pass

    try:
        ser.reset_input_buffer()
    except (OSError, serial.SerialException):
        pass

    try:
        ser.reset_output_buffer()
    except (OSError, serial.SerialException):
        pass

    try:
        ser.setDTR(False)
        ser.setRTS(False)
    except (AttributeError, OSError, serial.SerialException):
        pass

    try:
        time.sleep(0.2)
        ser.close()
    except (OSError, serial.SerialException):
        pass


def resolve_raw_values(args, assignments):
    cli_values = normalize_axis_values(
        args.axes,
        [getattr(args, f"a{axis}") for axis in range(1, AXIS_NUM + 1)],
    )
    cli_values = apply_axis_assignments(cli_values, assignments)

    if args.raw:
        return [clamp_u16(value) for value in cli_values]

    strokes = normalize_strokes(args.strokes)
    raw_values = [mm_to_raw(value, stroke) for value, stroke in zip(cli_values, strokes)]

    print("Input mm:", [float(f"{value:.3f}") for value in cli_values])
    print("Stroke mm:", [float(f"{value:.3f}") for value in strokes])
    return raw_values


def parse_interactive_line(line, raw_mode, strokes, current_values):
    text = line.strip()
    if not text:
        return None, current_values

    lowered = text.lower()
    if lowered in {"q", "quit", "exit"}:
        return "QUIT", current_values
    if lowered in {"z", "zero"}:
        values = [0.0] * AXIS_NUM
    else:
        if "=" in text:
            values = list(current_values)
            assignments = text.replace(",", " ").split()
            values = apply_axis_assignments(values, assignments)
            if raw_mode:
                return [clamp_u16(value) for value in values], values
            return [mm_to_raw(value, stroke) for value, stroke in zip(values, strokes)], values

        parts = text.replace(",", " ").split()
        if len(parts) > AXIS_NUM:
            raise ValueError(f"Please enter at most {AXIS_NUM} axis values.")
        values = [0.0] * AXIS_NUM
        for index, part in enumerate(parts):
            values[index] = float(part)

    if raw_mode:
        return [clamp_u16(value) for value in values], values

    return [mm_to_raw(value, stroke) for value, stroke in zip(values, strokes)], values


def run_interactive_mode(ser, args):
    strokes = normalize_strokes(args.strokes)
    mode_name = "raw uint16" if args.raw else "mm"
    current_values = [0.0] * AXIS_NUM

    print(f"Interactive mode started ({mode_name}).")
    print("Enter 1 to 7 values for A1..A7, separated by spaces.")
    print("You can also use assignments like a1=30 a2=15 a7=80.")
    print("Assignment mode keeps previous axis values. Type 'zero' to reset all axes, 'quit' to exit.")

    while True:
        try:
            line = input("flypt> ")
        except EOFError:
            print()
            break
        except KeyboardInterrupt:
            print()
            break

        try:
            result, next_values = parse_interactive_line(line, args.raw, strokes, current_values)
        except ValueError as exc:
            print(f"Input error: {exc}")
            continue

        if result is None:
            continue
        if result == "QUIT":
            break

        current_values = next_values
        if args.raw:
            print("Input raw:", [clamp_u16(value) for value in current_values])
        else:
            print("Input mm:", [float(f"{value:.3f}") for value in current_values])
        send_flypt_frame(ser, result)


def main():
    parser = build_parser()
    args, extra_args = parser.parse_known_args()

    if args.list_ports:
        list_serial_ports()
        return

    if not args.port:
        parser.error("Please provide --port, or use --list-ports to view available ports.")

    if args.repeat < 1:
        parser.error("--repeat must be >= 1.")
    if args.interval < 0:
        parser.error("--interval must be >= 0.")
    if args.hold_open < 0:
        parser.error("--hold-open must be >= 0.")

    raw_values = None
    if not args.interactive:
        try:
            raw_values = resolve_raw_values(args, extra_args)
        except ValueError as exc:
            parser.error(str(exc))
    elif extra_args:
        try:
            apply_axis_assignments([0.0] * AXIS_NUM, extra_args)
        except ValueError as exc:
            parser.error(str(exc))

    ser = None
    try:
        ser = open_serial_port(args)
        print(f"Serial opened: {ser.name} @ {args.baud}")
        print_serial_config(ser)
        if args.open_delay > 0:
            time.sleep(args.open_delay)

        if args.interactive:
            run_interactive_mode(ser, args)
        else:
            for index in range(args.repeat):
                send_flypt_frame(ser, raw_values)
                if index < args.repeat - 1 and args.interval > 0:
                    time.sleep(args.interval)

            if args.wait_exit:
                input("Press Enter to close the serial port...")
            elif args.hold_open > 0:
                print(f"Holding serial port open for {args.hold_open:.1f}s...")
                time.sleep(args.hold_open)
    finally:
        safe_close_serial(ser)

    print("Test done.")


if __name__ == "__main__":
    main()
