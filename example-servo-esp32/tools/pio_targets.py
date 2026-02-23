Import("env")

import os
import sys
import glob
import json
import importlib.util

try:
    from serial.tools import list_ports
except Exception:
    list_ports = None


def get_upload_port(env):
    for opt in ("upload_port", "manual_upload_port"):
        try:
            port = env.GetProjectOption(opt)
        except Exception:
            port = ""
        if port:
            return port

    try:
        port = env.AutodetectUploadPort()
        if isinstance(port, (list, tuple)):
            if not port:
                return ""
            first = port[0]
            if hasattr(first, "port"):
                return getattr(first, "port") or ""
            if isinstance(first, dict) and "port" in first:
                return first.get("port") or ""
            return first if isinstance(first, str) else ""
        if isinstance(port, dict) and "port" in port:
            return port.get("port") or ""
        if hasattr(port, "port"):
            return getattr(port, "port") or ""
        return port or ""
    except Exception:
        pass

    if list_ports:
        ports = [p.device for p in list_ports.comports() if p.device]
        preferred = [p for p in ports if "usbserial" in p or "usbmodem" in p]
        if preferred:
            return preferred[0]
        if ports:
            return ports[0]

    if sys.platform == "darwin":
        candidates = sorted(glob.glob("/dev/cu.*"))
        candidates = [p for p in candidates if "Bluetooth" not in p]
        preferred = [p for p in candidates if "usbserial" in p or "usbmodem" in p]
        if preferred:
            return preferred[0]
        if candidates:
            return candidates[0]
    return ""


def package_action(*args, **kwargs):
    project_dir = env.get("PROJECT_DIR") or env.subst("$PROJECT_DIR")
    post_path = os.path.join(project_dir, "tools", "pio_post.py")
    if not os.path.exists(post_path):
        print("[build:release] pio_post.py not found")
        return

    spec = importlib.util.spec_from_file_location("pio_post_build_release", post_path)
    if not spec or not spec.loader:
        print("[build:release] failed to load pio_post.py")
        return
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)

    fw_node = env.File(os.path.join(env.subst("$BUILD_DIR"), "firmware.bin"))
    mod.package_firmware([fw_node], [], env)
    if hasattr(mod, "rebuild_manifest_from_folders"):
        mod.rebuild_manifest_from_folders(project_dir)


def manual_upload_action(*args, **kwargs):
    port = ""
    if sys.platform == "darwin":
        candidates = sorted([p for p in glob.glob("/dev/cu.*") if "Bluetooth" not in p])
        preferred = [p for p in candidates if "usbserial" in p or "usbmodem" in p]
        print(f"[manual_upload] cu ports: {candidates}")
        print(f"[manual_upload] preferred: {preferred}")
        if preferred:
            port = preferred[0]
        elif candidates:
            port = candidates[0]
    if not port:
        port = get_upload_port(env)

    if not port:
        print("[manual_upload] missing upload port. Set upload_port in platformio.ini")
        return

    project_dir = env.get("PROJECT_DIR") or env.subst("$PROJECT_DIR")
    fw_root = os.path.join(project_dir, "web", "servo", "firmware")
    manifest_path = os.path.join(fw_root, "firmwares.json")
    if not os.path.exists(manifest_path):
        print("[manual_upload] firmwares.json not found. Build first.")
        return

    try:
        with open(manifest_path, "r", encoding="utf-8") as f:
            data = json.load(f)
    except Exception as err:
        print(f"[manual_upload] failed to read firmwares.json: {err}")
        return

    firmwares = data.get("firmwares") if isinstance(data, dict) else None
    if not firmwares:
        print("[manual_upload] firmwares.json empty")
        return

    fw = firmwares[0]
    base = fw.get("name", "")
    fw_dir = fw.get("dir", "")
    if not base:
        print("[manual_upload] invalid firmware name")
        return

    base_dir = os.path.join(fw_root, fw_dir) if fw_dir else fw_root
    boot = os.path.join(base_dir, f"{base}_bootloader.bin")
    part = os.path.join(base_dir, f"{base}_partitions.bin")
    boot_app0 = os.path.join(base_dir, f"{base}_boot_app0.bin")
    app = os.path.join(base_dir, f"{base}.bin")

    for path in (boot, part, boot_app0, app):
        if not os.path.exists(path):
            print(f"[manual_upload] missing file: {path}")
            return

    print(f"[manual_upload] port: {port}")
    print(f"[manual_upload] firmware: {base}")

    cmd = [
        "pio",
        "pkg",
        "exec",
        "-p",
        "tool-esptoolpy",
        "--",
        "esptool.py",
        "--chip",
        "esp32s3",
        "--port",
        port,
        "--baud",
        "460800",
        "--before",
        "default_reset",
        "--after",
        "hard_reset",
        "write_flash",
        "-z",
        "0x0000",
        boot,
        "0x8000",
        part,
        "0xE000",
        boot_app0,
        "0x10000",
        app,
    ]
    env.Execute(" ".join(cmd))


build_target = env.AddCustomTarget(
    name="build:release",
    dependencies=["$BUILD_DIR/firmware.bin"],
    actions=[package_action],
    title="Build (Release)",
    description="Build firmware (release) and generate packaged binaries",
)

env.AlwaysBuild(build_target)

env.AddCustomTarget(
    name="manual_upload",
    dependencies=[],
    actions=[manual_upload_action],
    title="Manual Upload (Latest)",
    description="Flash latest firmware from web/servo/firmware using esptool (auto port)",
)
