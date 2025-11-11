#include <iostream>
#include <fstream>
#include <map>
#include <stdexcept>
#include <yaml-cpp/yaml.h>

extern "C" {
    #include <stdio.h>
}

using namespace std;

struct WorldConfig {
    string world_name;
    string onboarding_name;
    string task_name;
    int max_time_in_pool;
    int agents_required;
    string backup_task;
};

map<string, WorldConfig> parse_configuration_file(const string& config_path) {
    map<string, WorldConfig> result;
    YAML::Node cfg = YAML::LoadFile(config_path);

    if (!cfg["world_module"]) throw runtime_error("Did not specify world module");
    if (!cfg["overworld"]) throw runtime_error("Did not specify overworld");
    if (!cfg["max_workers"]) throw runtime_error("Did not specify max_workers");
    if (!cfg["task_name"]) throw runtime_error("Did not specify task name");

    auto task_world = cfg["tasks"];
    if (!task_world || task_world.size() == 0) throw runtime_error("task not in config file");

    for (auto task : task_world) {
        auto task_name = task.first.as<string>();
        auto configuration = task.second;
        if (!configuration["task_world"]) {
            throw runtime_error(task_name + " does not specify a task");
        }

        WorldConfig config = {
            task_name,
            configuration["onboard_world"] ? configuration["onboard_world"].as<string>() : "",
            configuration["task_world"].as<string>(),
            configuration["timeout"] ? configuration["timeout"].as<int>() : 300,
            configuration["agents_required"] ? configuration["agents_required"].as<int>() : 1,
            configuration["backup_task"] ? configuration["backup_task"].as<string>() : ""
        };
        result[task_name] = config;
    }
    return result;
}

int main() {
    try {
        auto configs = parse_configuration_file("config.yml");
        // Do something with configs
        printf("Configuration parsed successfully.\n");
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }
    return 0;
}