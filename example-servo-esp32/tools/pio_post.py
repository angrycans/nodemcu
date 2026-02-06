try:
    Import('env')
except Exception:
    pass

import os
import re
import json
import shutil
from datetime import datetime


def read_fw_version(project_dir):
    src_path = os.path.join(project_dir, "src", "main.cpp")
    if not os.path.exists(src_path):
        return "0.0.0"
    with open(src_path, "r", encoding="utf-8") as f:
        content = f.read()
    match = re.search(r'FW_VERSION\s*=\s*"([^"]+)"', content)
    return match.group(1).strip() if match else "0.0.0"


def safe_copy(src, dst):
    if os.path.exists(src):
        shutil.copy2(src, dst)
        return True
    return False


def load_manifest(path):
    if not os.path.exists(path):
        return {"firmwares": []}
    try:
        with open(path, "r", encoding="utf-8") as f:
            data = json.load(f)
        if not isinstance(data, dict):
            return {"firmwares": []}
        if "firmwares" not in data or not isinstance(data["firmwares"], list):
            data["firmwares"] = []
        return data
    except Exception:
        return {"firmwares": []}


def save_manifest(path, data):
    with open(path, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=2)

def find_boot_app0(env):
    try:
        platform = env.PioPlatform()
        fw_dir = platform.get_package_dir("framework-arduinoespressif32")
        if fw_dir:
            cand = os.path.join(fw_dir, "tools", "partitions", "boot_app0.bin")
            if os.path.exists(cand):
                return cand
    except Exception:
        pass
    return ""


def package_firmware(target, source, env):
    project_dir = env.get("PROJECT_DIR") or env.subst("$PROJECT_DIR")
    if not project_dir:
        print("[fw] missing PROJECT_DIR")
        return

    if not target:
        print("[fw] missing source")
        return

    app_src = target[0].get_abspath()
    build_dir = os.path.dirname(app_src)

    fw_version = read_fw_version(project_dir)
    now = datetime.now()
    stamp = now.strftime("%Y%m%d_%H%M")
    base = f"{fw_version}_{stamp}"

    firmware_root = os.path.join(project_dir, "web", "servo", "firmware")
    os.makedirs(firmware_root, exist_ok=True)
    out_dir = os.path.join(firmware_root, fw_version)
    if os.path.isdir(out_dir):
        for name in os.listdir(out_dir):
            path = os.path.join(out_dir, name)
            if os.path.isfile(path) or os.path.islink(path):
                os.remove(path)
            elif os.path.isdir(path):
                shutil.rmtree(path)
    else:
        os.makedirs(out_dir, exist_ok=True)

    boot_src = os.path.join(build_dir, "bootloader.bin")
    part_src = os.path.join(build_dir, "partitions.bin")
    boot_app0_src = find_boot_app0(env)

    app_dst = os.path.join(out_dir, f"{base}.bin")
    boot_dst = os.path.join(out_dir, f"{base}_bootloader.bin")
    part_dst = os.path.join(out_dir, f"{base}_partitions.bin")
    boot_app0_dst = os.path.join(out_dir, f"{base}_boot_app0.bin")

    if not os.path.exists(app_src):
        print("[fw] firmware.bin not found, skip")
        return

    safe_copy(app_src, app_dst)
    boot_ok = safe_copy(boot_src, boot_dst)
    part_ok = safe_copy(part_src, part_dst)
    boot_app0_ok = safe_copy(boot_app0_src, boot_app0_dst) if boot_app0_src else False

    parts = []
    if boot_ok:
        parts.append({"path": f"{base}_bootloader.bin", "offset": 0})
    if part_ok:
        parts.append({"path": f"{base}_partitions.bin", "offset": 32768})
    if boot_app0_ok:
        parts.append({"path": f"{base}_boot_app0.bin", "offset": 57344})
    parts.append({"path": f"{base}.bin", "offset": 65536})

    install_manifest = {
        "name": "Acans ESP32 7Axis Firmware",
        "version": fw_version,
        "new_install_improv_wait_time": 0,
        "builds": [
            {
                "chipFamily": "ESP32-S3",
                "parts": parts
            }
        ]
    }

    install_name = f"install-{base}.json"
    install_path = os.path.join(out_dir, install_name)
    with open(install_path, "w", encoding="utf-8") as f:
        json.dump(install_manifest, f, ensure_ascii=False, indent=2)

    manifest_path = os.path.join(firmware_root, "firmwares.json")
    manifest = load_manifest(manifest_path)

    entry = {
        "name": base,
        "version": fw_version,
        "date": now.strftime("%Y-%m-%d %H:%M"),
        "ts": int(now.timestamp()),
        "dir": fw_version,
        "bin": f"{base}.bin",
        "installManifest": install_name
    }
    manifest["firmwares"] = [e for e in manifest.get("firmwares", []) if e.get("name") != base]
    manifest["firmwares"].append(entry)
    manifest["firmwares"].sort(key=lambda x: x.get("ts", 0), reverse=True)

    save_manifest(manifest_path, manifest)
    print(f"[fw] packaged: {base}")


# 打包只由 build:release 触发，不在常规 build 时自动执行
