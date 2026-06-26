Import("env")

# PlatformIO forces compressed writes with "-z", even when esptool uses the
# ESP32-S3 ROM loader via --no-stub.
upload_flags = [
    "--no-compress" if flag == "-z" else flag
    for flag in env["UPLOADERFLAGS"]
]
env.Replace(
    UPLOADER=env.File("esptool_small_blocks.py").get_abspath(),
    UPLOADERFLAGS=upload_flags,
)
