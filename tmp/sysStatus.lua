--system status
print(
    string.format(
        '\nNodemcu wifi mode\n\twifi.NULLMODE:"%s"\n\twifi.STATION:"%s"\n\twifi.SOFTAP:"%s"\n\twifi.STATIONAP:"%s"',
        wifi.NULLMODE,
        wifi.STATION,
        wifi.SOFTAP,
        wifi.STATIONAP
    )
)
print(string.format('\ncurrent wifi mode:"%s"\n\t', wifi.getmode()))

local sta_config = wifi.sta.getconfig(true)
print(
    string.format(
        '\tCurrent station config\n\tssid:"%s"\tpassword:"%s"\n\tbssid:"%s"\tbssid_set:%s',
        sta_config.ssid,
        sta_config.pwd,
        sta_config.bssid,
        (sta_config.bssid_set and "true" or "false")
    )
)

print(string.format('\ncurrent wifi ip:"%s"', wifi.sta.getip()))

function getSysStatus()
    local stable = {}
    local sta_config = wifi.sta.getconfig(true)
    stable["wifi_mode"] = wifi.getmode()
    stable["wifi_ssid"] = {
        ssid = sta_config.ssid,
        bssid = sta_config.bssid
    }

    local ip, mask, gate = wifi.sta.getip()
    stable["wifi_ip"] = {ip = ip, mask = mask, gate = gate}

    return stable
end
