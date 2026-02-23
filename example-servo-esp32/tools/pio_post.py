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


def read_build_variant(project_dir):
    src_path = os.path.join(project_dir, "src", "main.cpp")
    if not os.path.exists(src_path):
        return ""
    with open(src_path, "r", encoding="utf-8") as f:
        lines = f.readlines()
    has_servo = False
    has_stepper = False
    for line in lines:
        if re.match(r'^\s*#define\s+USE_SERVO\b', line):
            has_servo = True
        if re.match(r'^\s*#define\s+USE_STEPPER\b', line):
            has_stepper = True
    if has_stepper and not has_servo:
        return "stepper"
    if has_servo and not has_stepper:
        return "servo"
    return ""


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


def parse_base_timestamp(base):
    match = re.search(r"_(\d{8})_(\d{4})$", base)
    if not match:
        return None
    try:
        return datetime.strptime(f"{match.group(1)}{match.group(2)}", "%Y%m%d%H%M")
    except Exception:
        return None


def build_entry_from_install(firmware_root, dir_name, install_name):
    if not install_name.startswith("install-") or not install_name.endswith(".json"):
        return None
    base = install_name[len("install-"):-len(".json")]
    if not base:
        return None

    base_dir = os.path.join(firmware_root, dir_name) if dir_name else firmware_root
    install_path = os.path.join(base_dir, install_name)
    app = os.path.join(base_dir, f"{base}.bin")
    boot = os.path.join(base_dir, f"{base}_bootloader.bin")
    part = os.path.join(base_dir, f"{base}_partitions.bin")
    boot_app0 = os.path.join(base_dir, f"{base}_boot_app0.bin")

    required = [install_path, app, boot, part, boot_app0]
    if not all(os.path.exists(p) for p in required):
        return None

    version = dir_name
    try:
        with open(install_path, "r", encoding="utf-8") as f:
            data = json.load(f)
        if isinstance(data, dict) and data.get("version"):
            version = str(data.get("version")).strip()
    except Exception:
        pass

    ts_dt = parse_base_timestamp(base)
    if ts_dt:
        ts = int(ts_dt.timestamp())
        date_str = ts_dt.strftime("%Y-%m-%d %H:%M")
    else:
        ts = int(os.path.getmtime(install_path))
        date_str = datetime.fromtimestamp(ts).strftime("%Y-%m-%d %H:%M")

    return {
        "name": base,
        "version": version or dir_name,
        "date": date_str,
        "ts": ts,
        "dir": dir_name,
        "bin": f"{base}.bin",
        "installManifest": install_name,
    }


def rebuild_manifest_from_folders(project_dir):
    firmware_root = os.path.join(project_dir, "web", "servo", "firmware")
    if not os.path.isdir(firmware_root):
        print("[fw] firmware folder missing, skip rebuild")
        return

    entries = []
    for dir_name in sorted(os.listdir(firmware_root)):
        if not dir_name or dir_name.startswith("."):
            continue
        dir_path = os.path.join(firmware_root, dir_name)
        if not os.path.isdir(dir_path):
            continue
        for filename in sorted(os.listdir(dir_path)):
            if not filename.startswith("install-") or not filename.endswith(".json"):
                continue
            entry = build_entry_from_install(firmware_root, dir_name, filename)
            if entry:
                entries.append(entry)

    deduped = {}
    for entry in entries:
        deduped[entry["name"]] = entry

    manifest = {"firmwares": sorted(deduped.values(), key=lambda x: x.get("ts", 0), reverse=True)}
    manifest_path = os.path.join(firmware_root, "firmwares.json")
    save_manifest(manifest_path, manifest)
    print(f"[fw] manifest rebuilt: {len(manifest['firmwares'])} entries")

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
    variant = read_build_variant(project_dir)
    variant_suffix = f"_{variant}" if variant else ""
    version_tag = f"{fw_version}{variant_suffix}"
    now = datetime.now()
    stamp = now.strftime("%Y%m%d_%H%M")
    base = f"{version_tag}_{stamp}"

    firmware_root = os.path.join(project_dir, "web", "servo", "firmware")
    os.makedirs(firmware_root, exist_ok=True)
    out_dir = os.path.join(firmware_root, version_tag)
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
        "version": version_tag,
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
        "version": version_tag,
        "date": now.strftime("%Y-%m-%d %H:%M"),
        "ts": int(now.timestamp()),
        "dir": version_tag,
        "bin": f"{base}.bin",
        "installManifest": install_name
    }
    manifest["firmwares"] = [e for e in manifest.get("firmwares", []) if e.get("name") != base]
    manifest["firmwares"].append(entry)
    manifest["firmwares"].sort(key=lambda x: x.get("ts", 0), reverse=True)

    save_manifest(manifest_path, manifest)
    print(f"[fw] packaged: {base}")


# 打包只由 build:release 触发，不在常规 build 时自动执行
