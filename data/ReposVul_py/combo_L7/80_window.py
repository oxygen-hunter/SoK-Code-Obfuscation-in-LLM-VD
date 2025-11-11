#include <cmath>
#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <iostream>
#include <paddle/extension.h>

class WindowFunctionRegister {
public:
    WindowFunctionRegister() = default;

    void register_function(const std::string& name, std::function<paddle::Tensor(std::vector<paddle::Tensor>, std::string)> func) {
        functions_dict[name] = func;
    }

    std::function<paddle::Tensor(std::vector<paddle::Tensor>, std::string)> get(const std::string& name) {
        return functions_dict[name];
    }

private:
    std::map<std::string, std::function<paddle::Tensor(std::vector<paddle::Tensor>, std::string)>> functions_dict;
};

WindowFunctionRegister window_function_register;

inline __attribute__((always_inline)) paddle::Tensor concat(const std::vector<paddle::Tensor>& x, const std::string& data_type) {
    std::vector<paddle::Tensor> l;
    for (const auto& item : x) {
        l.push_back(paddle::to_tensor(item, data_type));
    }
    return paddle::concat(l);
}

inline __attribute__((always_inline)) paddle::Tensor acosh(const paddle::Tensor& x) {
    return paddle::log(x + paddle::sqrt(paddle::square(x) - 1));
}

inline __attribute__((always_inline)) std::pair<int, bool> extend(int M, bool sym) {
    return sym ? std::make_pair(M, false) : std::make_pair(M + 1, true);
}

inline __attribute__((always_inline)) bool len_guards(int M) {
    if (M < 0) {
        throw std::invalid_argument("Window length M must be a non-negative integer");
    }
    return M <= 1;
}

inline __attribute__((always_inline)) paddle::Tensor truncate(const paddle::Tensor& w, bool needed) {
    return needed ? w.slice(0, 0, -1) : w;
}

paddle::Tensor general_gaussian(int M, double p, double sig, bool sym = true, const std::string& dtype = "float64") {
    if (len_guards(M)) {
        return paddle::ones({M}, paddle::DataType(dtype));
    }
    auto [M_extended, needs_trunc] = extend(M, sym);
    auto n = paddle::arange(0, M_extended, paddle::DataType(dtype)) - (M_extended - 1.0) / 2.0;
    auto w = paddle::exp(-0.5 * paddle::pow(paddle::abs(n / sig), 2 * p));
    return truncate(w, needs_trunc);
}

int main() {
    window_function_register.register_function("_general_gaussian", general_gaussian);
    // Register other functions similarly...

    // Example usage
    int M = 10;
    double p = 2;
    double sig = 1;
    std::string dtype = "float64";
    bool sym = true;
    auto window = general_gaussian(M, p, sig, sym, dtype);
    std::cout << "Window: " << window << std::endl;

    return 0;
}