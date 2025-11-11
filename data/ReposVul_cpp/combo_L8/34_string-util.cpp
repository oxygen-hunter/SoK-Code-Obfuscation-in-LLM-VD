#include <Python.h>
#include <dlfcn.h>
#include <string>
#include <vector>
#include <iostream>

extern "C" {
  typedef const char* (*string_pad_t)(const char*, int, int, const char*, int, int);
  typedef const char* (*string_strip_tags_t)(const char*, int, const char*, int, bool);
}

class StringUtil {
public:
  static std::string Pad(const std::string& input, int final_length, const std::string& pad_string = " ", int type = 1) {
    void* handle = dlopen("libstringutil.so", RTLD_LAZY);
    if (!handle) {
      std::cerr << "Cannot load library: " << dlerror() << '\n';
      return std::string();
    }
    string_pad_t string_pad = (string_pad_t) dlsym(handle, "string_pad");
    const char* result = string_pad(input.c_str(), input.size(), final_length, pad_string.c_str(), pad_string.size(), type);
    std::string res_str(result);
    dlclose(handle);
    return res_str;
  }

  static std::string StripHTMLTags(const std::string& input, const std::string& allowable_tags = "") {
    void* handle = dlopen("libstringutil.so", RTLD_LAZY);
    if (!handle) {
      std::cerr << "Cannot load library: " << dlerror() << '\n';
      return std::string();
    }
    string_strip_tags_t string_strip_tags = (string_strip_tags_t) dlsym(handle, "string_strip_tags");
    const char* result = string_strip_tags(input.c_str(), input.size(), allowable_tags.c_str(), allowable_tags.size(), false);
    std::string res_str(result);
    dlclose(handle);
    return res_str;
  }
};

int main() {
  Py_Initialize();
  std::string padded = StringUtil::Pad("Hello", 10, "*");
  std::cout << "Padded string: " << padded << std::endl;

  std::string stripped = StringUtil::StripHTMLTags("<b>Hello</b>", "");
  std::cout << "Stripped string: " << stripped << std::endl;

  Py_Finalize();
  return 0;
}