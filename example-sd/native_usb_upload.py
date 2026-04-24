import os
import termios
import time

from serial import Serial, SerialException
from serial.tools import list_ports

Import("env")


BOOTLOADER_VID = 0x303A
APP_PORT_WAIT_SECONDS = 10.0
TOUCH_RETRY_SECONDS = 5.0
TOUCH_RETRY_INTERVAL = 0.25
BOOTLOADER_WAIT_SECONDS = 8.0
BOOTLOADER_POLL_INTERVAL = 0.1
BOOTLOADER_SETTLE_SECONDS = 1.0


def _as_int(value):
    return int(str(value), 0)


def _load_app_usb_ids():
    cpp_defines = env.get("CPPDEFINES", [])
    define_map = {}

    for item in cpp_defines:
        if isinstance(item, tuple) and len(item) == 2:
            define_map[item[0]] = item[1]
        elif isinstance(item, str):
            define_map[item] = True

    try:
        return _as_int(define_map["APP_USB_VID"]), _as_int(define_map["APP_USB_PID"])
    except KeyError as error:
        raise RuntimeError("Unable to read APP_USB_VID/APP_USB_PID from build_flags") from error


def _snapshot_ports():
    return {port.device: port for port in list_ports.comports()}


def _describe_port(port):
    parts = [port.device]
    if port.vid is not None and port.pid is not None:
        parts.append(f"VID:PID={port.vid:04X}:{port.pid:04X}")

    extras = []
    for value in (port.manufacturer, port.product, port.description):
        if value and value not in extras:
            extras.append(value)
    if extras:
        parts.append(" | ".join(extras))
    return " ".join(parts)


def _is_bootloader_port(port):
    # 乐鑫官方 Bootloader/JTAG 典型的 PID
    return port.vid == 0x303A and port.pid in (0x1001, 0x0002, 0x0009)

def _find_existing_bootloader_port(ports):
    for device, port in ports.items():
        if _is_bootloader_port(port):
            return device
    return None
def _find_app_port(ports, requested_port, app_vid, app_pid):
    if requested_port and requested_port in ports:
        return requested_port

    # 查找符合应用 VID/PID 的端口
    matches = [device for device, port in ports.items() if port.vid == app_vid and port.pid == app_pid]
    
    # 如果找到了，排除掉已经被识别为 Bootloader 的端口
    app_matches = [m for m in matches if not _is_bootloader_port(ports[m])]
    
    if len(app_matches) >= 1:
        return app_matches[0]

    if not matches:
        raise RuntimeError(f"No CDC port found with VID:PID={app_vid:04X}:{app_pid:04X}")

    return matches[0]


def _wait_for_app_port(requested_port, app_vid, app_pid):
    deadline = time.time() + APP_PORT_WAIT_SECONDS
    last_error = None

    while time.time() < deadline:
        current_ports = _snapshot_ports()
        try:
            return current_ports, _find_app_port(current_ports, requested_port, app_vid, app_pid)
        except RuntimeError as error:
            last_error = error
            time.sleep(BOOTLOADER_POLL_INTERVAL)

    if last_error is not None:
        raise last_error
    raise RuntimeError("No application CDC port found for the current USB MSC firmware.")


def _touch_port_1200bps(initial_port_name, requested_port, app_vid, app_pid):
    deadline = time.time() + TOUCH_RETRY_SECONDS
    current_port_name = initial_port_name
    last_error = None

    while time.time() < deadline:
        current_ports = _snapshot_ports()
        bootloader_port = _find_existing_bootloader_port(current_ports)
        if bootloader_port and bootloader_port != current_port_name:
            print(f"Native USB upload: bootloader port appeared early on {bootloader_port}")
            return bootloader_port

        if current_port_name not in current_ports:
            try:
                current_port_name = _find_app_port(current_ports, requested_port, app_vid, app_pid)
                print(f"Native USB upload: application CDC port moved to {current_port_name}")
            except RuntimeError as error:
                last_error = error
                time.sleep(TOUCH_RETRY_INTERVAL)
                continue

        print(f"Native USB upload: touching {current_port_name} at 1200 bps")
        try:
            with Serial(current_port_name, 1200, timeout=0.1, write_timeout=0.1, exclusive=False) as serial_port:
                serial_port.dtr = False
                serial_port.rts = False
                time.sleep(0.1)
            return None
        except (OSError, SerialException, termios.error) as error:
            last_error = error
            current_ports = _snapshot_ports()
            bootloader_port = _find_existing_bootloader_port(current_ports)
            if bootloader_port and bootloader_port != current_port_name:
                print(f"Native USB upload: bootloader port appeared during touch on {bootloader_port}")
                return bootloader_port

            if current_port_name not in current_ports:
                print(
                    f"Native USB upload: {current_port_name} disappeared during touch, "
                    "assuming reboot is already in progress"
                )
                return None

            print(f"Native USB upload: touch failed on {current_port_name}: {error}; retrying")
            time.sleep(TOUCH_RETRY_INTERVAL)

    if last_error is not None:
        raise RuntimeError(
            "Unable to trigger bootloader via the application CDC port.\n"
            f"Last touch error: {last_error}"
        )
    raise RuntimeError("Unable to trigger bootloader via the application CDC port.")


def _wait_for_bootloader_candidate(candidate, app_port):
    deadline = time.time() + BOOTLOADER_SETTLE_SECONDS

    while time.time() < deadline:
        current_ports = _snapshot_ports()
        port = current_ports.get(candidate)
        if port is None:
            return None
        if not _is_bootloader_port(port) and candidate == app_port:
            return None

        time.sleep(BOOTLOADER_POLL_INTERVAL)

    return candidate


def _wait_for_bootloader_port(before_ports, app_port):
    deadline = time.time() + BOOTLOADER_WAIT_SECONDS
    while time.time() < deadline:
        current_ports = _snapshot_ports()
        added_ports = [device for device in current_ports if device not in before_ports]

        for device in added_ports:
            port = current_ports[device]
            if _is_bootloader_port(port) or device != app_port:
                settled_device = _wait_for_bootloader_candidate(device, app_port)
                if settled_device:
                    return settled_device

        bootloader_port = _find_existing_bootloader_port(current_ports)
        if bootloader_port and bootloader_port != app_port:
            settled_device = _wait_for_bootloader_candidate(bootloader_port, app_port)
            if settled_device:
                return settled_device

        time.sleep(BOOTLOADER_POLL_INTERVAL)

    current_ports = _snapshot_ports()
    visible_ports = "\n".join(f"  - {_describe_port(port)}" for port in current_ports.values()) or "  (none)"
    raise RuntimeError(
        "Timed out waiting for the ESP32-S3 bootloader port to appear.\n"
        "Close any serial monitor and try again.\n"
        f"Visible serial ports:\n{visible_ports}"
    )


def _replace_flag_value(flags, option, value):
    updated = list(flags)
    for index, flag in enumerate(updated[:-1]):
        if flag == option:
            updated[index + 1] = value
            return updated
    updated.extend([option, value])
    return updated


def _prepare_esptool_flags():
    flags = list(env.get("UPLOADERFLAGS", []))
    # Using 'usb_reset' for native USB which is more reliable than 'no_reset' on some boards
    flags = _replace_flag_value(flags, "--before", "usb_reset")
    flags = _replace_flag_value(flags, "--after", "hard_reset")
    env.Replace(UPLOADERFLAGS=flags)
    os.environ.setdefault("ESPTOOL_OPEN_PORT_ATTEMPTS", "100")


def _before_upload(source, target, **kwargs):
    app_vid, app_pid = _load_app_usb_ids()
    before_ports = _snapshot_ports()
    requested_port = env.get("UPLOAD_PORT")

    if requested_port and requested_port in before_ports and _is_bootloader_port(before_ports[requested_port]):
        bootloader_port = requested_port
        print(f"Native USB upload: using existing bootloader port {bootloader_port} ({_describe_port(before_ports[bootloader_port])})")
    else:
        bootloader_port = _find_existing_bootloader_port(before_ports)
        if bootloader_port:
            print(f"Native USB upload: found bootloader port {bootloader_port} ({_describe_port(before_ports[bootloader_port])})")
            time.sleep(BOOTLOADER_SETTLE_SECONDS)
        else:
            before_ports, app_port = _wait_for_app_port(requested_port, app_vid, app_pid)
            print(f"Native USB upload: application CDC port is {app_port}")
            bootloader_port = _touch_port_1200bps(app_port, requested_port, app_vid, app_pid)
            if not bootloader_port:
                bootloader_port = _wait_for_bootloader_port(before_ports, app_port)

    _prepare_esptool_flags()
    env.Replace(UPLOAD_PORT=bootloader_port)
    print(f"Native USB upload: bootloader port is {bootloader_port}")


env.AddPreAction("upload", _before_upload)
