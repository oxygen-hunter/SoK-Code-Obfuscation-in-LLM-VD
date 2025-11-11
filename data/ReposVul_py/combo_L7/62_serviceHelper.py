#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <dbus/dbus.h>

#define UDEV_RULES_PATH "/etc/udev/rules.d/99-backintime-%s.rules"

class InvalidChar : public std::exception {
    const char* what() const throw() {
        return "Invalid character in parameter";
    }
};

class InvalidCmd : public std::exception {
    const char* what() const throw() {
        return "Invalid command in parameter";
    }
};

class LimitExceeded : public std::exception {
    const char* what() const throw() {
        return "Limit exceeded";
    }
};

class PermissionDeniedByPolicy : public std::exception {
    const char* what() const throw() {
        return "Permission denied by policy";
    }
};

class UdevRules {
public:
    UdevRules(DBusConnection* conn, const char* object_path, const char* bus_name) {
        // Initialization code
    }

    void addRule(const std::string& cmd, const std::string& uuid) {
        std::regex char_regex("[^a-zA-Z0-9-/\\.>& ]");
        std::smatch char_match;
        if (std::regex_search(cmd, char_match, char_regex)) {
            throw InvalidChar();
        }

        std::regex uuid_regex("[^a-zA-Z0-9-]");
        std::smatch uuid_match;
        if (std::regex_search(uuid, uuid_match, uuid_regex)) {
            throw InvalidChar();
        }

        validateCmd(cmd);

        std::string user = connectionUnixUser();
        std::string owner = nameOwner();

        checkLimits(owner, cmd);

        std::string sucmd = "su - '" + user + "' -c '" + cmd + "'";
        std::string rule = "ACTION==\"add|change\", ENV{ID_FS_UUID}==\"" + uuid + "\", RUN+=\"" + sucmd + "\"\n";

        if (tmpDict.find(owner) == tmpDict.end()) {
            tmpDict[owner] = std::vector<std::string>();
        }
        tmpDict[owner].push_back(rule);
    }

    bool save() {
        std::string user = connectionUnixUser();
        std::string owner = nameOwner();

        if (tmpDict.find(owner) == tmpDict.end() || tmpDict[owner].empty()) {
            deleteRule();
            return false;
        }

        std::string path = std::string(UDEV_RULES_PATH).replace("%s", user.c_str());
        if (access(path.c_str(), F_OK) == 0) {
            std::ifstream file(path);
            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            if (tmpDict[owner] == content) {
                clean(owner);
                return false;
            }
        }

        checkPolkitPrivilege("net.launchpad.backintime.UdevRuleSave");
        std::ofstream file(path);
        for (const auto& rule : tmpDict[owner]) {
            file << rule;
        }
        clean(owner);
        return true;
    }

    void deleteRule() {
        std::string user = connectionUnixUser();
        std::string owner = nameOwner();
        clean(owner);
        std::string path = std::string(UDEV_RULES_PATH).replace("%s", user.c_str());
        if (access(path.c_str(), F_OK) == 0) {
            checkPolkitPrivilege("net.launchpad.backintime.UdevRuleDelete");
            remove(path.c_str());
        }
    }

private:
    std::map<std::string, std::vector<std::string>> tmpDict;
    std::string su;
    std::string backintime;
    std::string nice;
    std::string ionice;
    int max_rules = 100;
    int max_users = 20;
    int max_cmd_len = 100;

    void validateCmd(const std::string& cmd) {
        if (cmd.find("&&") != std::string::npos) {
            throw InvalidCmd();
        } else if (cmd[0] != '/') {
            throw InvalidCmd();
        }

        std::vector<std::string> parts;
        std::stringstream ss(cmd);
        std::string part;
        while (ss >> part) {
            parts.push_back(part);
        }

        std::vector<std::pair<std::string, std::vector<std::string>>> whitelist = {
            {nice, {"-n"}},
            {ionice, {"-c", "-n"}}
        };

        for (const auto& entry : whitelist) {
            if (!parts.empty() && parts[0] == entry.first) {
                parts.erase(parts.begin());
                for (const auto& switch_opt : entry.second) {
                    while (!parts.empty() && parts[0].find(switch_opt) == 0) {
                        parts.erase(parts.begin());
                    }
                }
            }
        }

        if (parts.empty()) {
            throw InvalidCmd();
        } else if (parts[0] != backintime) {
            throw InvalidCmd();
        }
    }

    void checkLimits(const std::string& owner, const std::string& cmd) {
        if (tmpDict[owner].size() >= max_rules) {
            throw LimitExceeded();
        } else if (tmpDict.size() >= max_users) {
            throw LimitExceeded();
        } else if (cmd.size() > max_cmd_len) {
            throw LimitExceeded();
        }
    }

    std::string connectionUnixUser() {
        uid_t uid = getuid();
        struct passwd* pw = getpwuid(uid);
        return pw->pw_name;
    }

    std::string nameOwner() {
        // Logic to get name owner
        return "owner";
    }

    void clean(const std::string& owner) {
        if (tmpDict.find(owner) != tmpDict.end()) {
            tmpDict.erase(owner);
        }
    }

    void checkPolkitPrivilege(const std::string& privilege) {
        // Logic to check policy kit privilege
    }
};

int main() {
    DBusConnection* connection = dbus_bus_get(DBUS_BUS_SYSTEM, nullptr);
    UdevRules object(connection, "/UdevRules", "net.launchpad.backintime.serviceHelper");

    std::cout << "Running BIT service." << std::endl;

    while (true) {
        // Event loop
    }

    return 0;
}