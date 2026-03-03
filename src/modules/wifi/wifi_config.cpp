#include "wifi_config.h"

#include <cctype>

static std::string trimCopy(const std::string& input) {
    size_t begin = 0;
    while (begin < input.size() && std::isspace(static_cast<unsigned char>(input[begin])) != 0) {
        begin++;
    }

    size_t end = input.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(input[end - 1])) != 0) {
        end--;
    }

    return input.substr(begin, end - begin);
}

static std::string toLowerCopy(std::string value) {
    for (char& c : value) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return value;
}

static std::string unquoteValue(std::string value) {
    value = trimCopy(value);
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
        value = value.substr(1, value.size() - 2);
    }
    return value;
}

bool parseWifiConfigText(
    const std::string& rawConfig,
    const WifiConfigDefaults& defaults,
    WifiConfigData& out
) {
    out = WifiConfigData{};
    out.apSsid = defaults.apSsid;
    out.apPassword = defaults.apPassword;

    bool sawMode = false;
    size_t start = 0;
    while (start <= rawConfig.size()) {
        size_t end = rawConfig.find('\n', start);
        if (end == std::string::npos) {
            end = rawConfig.size();
        }
        std::string line = rawConfig.substr(start, end - start);
        start = end + 1;

        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        line = trimCopy(line);

        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }

        const size_t eqPos = line.find('=');
        if (eqPos == std::string::npos || eqPos == 0) {
            continue;
        }

        std::string key = toLowerCopy(trimCopy(line.substr(0, eqPos)));
        std::string value = unquoteValue(line.substr(eqPos + 1));

        if (key == "mode") {
            std::string mode = toLowerCopy(value);
            if (mode == "ap") {
                out.mode = WifiConfigMode::AP;
                sawMode = true;
            } else if (mode == "sta") {
                out.mode = WifiConfigMode::STA;
                sawMode = true;
            } else if (mode == "auto") {
                out.mode = WifiConfigMode::AUTO;
                sawMode = true;
            }
            continue;
        }

        if (key == "sta_ssid") {
            out.staSsid = value;
        } else if (key == "sta_password") {
            out.staPassword = value;
        } else if (key == "ap_ssid") {
            out.apSsid = value;
        } else if (key == "ap_password") {
            out.apPassword = value;
        }
    }

    if (!sawMode) {
        return false;
    }

    if (out.apSsid.empty()) {
        out.apSsid = defaults.apSsid;
    }
    if (out.apPassword.size() < 8) {
        out.apPassword = defaults.apPassword;
    }

    return true;
}
