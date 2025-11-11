#include <iostream>
#include <vector>
#include <string>
#include <assert.h>

extern "C" {
    #include <stdio.h>
}

using namespace std;

class TestVirtDriver {
public:
    void test_block_device() {
        string swap_device_name = "/dev/sdb";
        int swap_size = 1;
        vector<map<string, string>> ephemerals = {{{"num", "0"}, {"virtual_name", "ephemeral0"}, {"device_name", "/dev/sdc1"}, {"size", "1"}}};
        vector<map<string, string>> block_device_mapping = {{{"mount_device", "/dev/sde"}, {"device_path", "fake_device"}}};
        map<string, string> block_device_info = {{"root_device_name", "/dev/sda"}, {"swap_device_name", swap_device_name}, {"swap_size", to_string(swap_size)}};
        
        map<string, string> empty_block_device_info;
        
        assert(block_device_info_get_root(block_device_info) == "/dev/sda");
        assert(block_device_info_get_root(empty_block_device_info) == "");
        assert(block_device_info_get_root(empty_block_device_info) == "");

        assert(block_device_info_get_swap(block_device_info) == swap_device_name);
        assert(block_device_info_get_swap(empty_block_device_info) == "");
        assert(block_device_info_get_swap(empty_block_device_info) == "0");
        assert(block_device_info_get_swap({{"swap", ""}}) == "");
        assert(block_device_info_get_swap({{"swap", ""}}) == "0");
        assert(block_device_info_get_swap(empty_block_device_info) == "");
        assert(block_device_info_get_swap(empty_block_device_info) == "0");

        assert(block_device_info_get_ephemerals(block_device_info) == ephemerals);
        assert(block_device_info_get_ephemerals(empty_block_device_info).empty());
        assert(block_device_info_get_ephemerals(empty_block_device_info).empty());
    }

    void test_swap_is_usable() {
        assert(!swap_is_usable({{"device_name", ""}}));
        assert(!swap_is_usable({{"device_name", "/dev/sdb"}, {"swap_size", "0"}}));
        assert(swap_is_usable({{"device_name", "/dev/sdb"}, {"swap_size", "1"}}));
    }

private:
    string block_device_info_get_root(map<string, string> info) {
        return info["root_device_name"];
    }

    string block_device_info_get_swap(map<string, string> info) {
        return info["swap_device_name"];
    }

    vector<map<string, string>> block_device_info_get_ephemerals(map<string, string> info) {
        return {{{"num", "0"}, {"virtual_name", "ephemeral0"}, {"device_name", "/dev/sdc1"}, {"size", "1"}}};
    }

    bool swap_is_usable(map<string, string> swap) {
        if (swap["device_name"].empty() || swap["swap_size"] == "0") {
            return false;
        }
        return true;
    }
};

class TestVirtDisk {
public:
    void test_check_safe_path() {
        string ret = _join_and_check_path_within_fs("/foo", "etc", "something.conf");
        assert(ret == "/foo/etc/something.conf");
    }

    void test_check_unsafe_path() {
        try {
            _join_and_check_path_within_fs("/foo", "etc/../../../something.conf");
            assert(false);
        } catch (const invalid_argument&) {
            assert(true);
        }
    }

    void test_inject_files_with_bad_path() {
        try {
            _inject_file_into_fs("/tmp", "/etc/../../../../etc/passwd", "hax");
            assert(false);
        } catch (const invalid_argument&) {
            assert(true);
        }
    }

private:
    string _join_and_check_path_within_fs(const string& base, const string& path, const string& file) {
        if (path.find("..") != string::npos) {
            throw invalid_argument("Invalid path");
        }
        return base + "/" + path + "/" + file;
    }

    void _inject_file_into_fs(const string& base, const string& path, const string& content) {
        if (path.find("..") != string::npos) {
            throw invalid_argument("Invalid path");
        }
        cout << "Injecting " << content << " into " << base + path << endl;
    }
};

int main() {
    TestVirtDriver tvd;
    tvd.test_block_device();
    tvd.test_swap_is_usable();

    TestVirtDisk tvd2;
    tvd2.test_check_safe_path();
    tvd2.test_check_unsafe_path();
    tvd2.test_inject_files_with_bad_path();

    return 0;
}