#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <variant>
#include <functional>

// Helper functions for variant handling
std::string variantToString(const std::variant<int, std::string, bool>& v) {
    return std::visit([](const auto& value) -> std::string {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, int>) {
            return std::to_string(value);
        } else if constexpr (std::is_same_v<T, std::string>) {
            return value;
        } else if constexpr (std::is_same_v<T, bool>) {
            return value ? "true" : "false";
        } else {
            return "<unknown>";
        }
    }, v);
}

std::ostream& operator<<(std::ostream& os, const std::variant<int, std::string, bool>& v) {
    os << variantToString(v);
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<std::variant<int, std::string, bool>>& vec) {
    os << '[';
    for (size_t i = 0; i < vec.size(); ++i) {
        os << vec[i];
        if (i < vec.size() - 1) {
            os << ", ";
        }
    }
    os << ']';
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<std::string>& vec) {
    os << '[';
    for (size_t i = 0; i < vec.size(); ++i) {
        os << '"' << vec[i] << '"';
        if (i < vec.size() - 1) {
            os << ", ";
        }
    }
    os << ']';
    return os;
}

std::string stringReplace(const std::string& str, const std::string& oldStr, const std::string& newStr) {
    std::string result = str;
    size_t pos = 0;
    while ((pos = result.find(oldStr, pos)) != std::string::npos) {
        result.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }
    return result;
}

std::vector<std::string> stringExcision(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> result;
    size_t start = 0;
    size_t end = str.find(delimiter);
    while (end != std::string::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }
    result.push_back(str.substr(start));
    return result;
}

void listAdd(std::vector<std::variant<int, std::string, bool>>& list, int value) {
    list.push_back(value);
}

void listAdd(std::vector<std::variant<int, std::string, bool>>& list, const std::string& value) {
    list.push_back(value);
}

void listAdd(std::vector<std::variant<int, std::string, bool>>& list, const char* value) {
    list.push_back(std::string(value));
}

void listAdd(std::vector<std::variant<int, std::string, bool>>& list, bool value) {
    list.push_back(value);
}

char get(const std::string& str, int index) {
    if (index < 0) {
        index = str.length() + index;
    }
    if (index < 0 || index >= str.length()) {
        return '\0';
    }
    return str[index];
}

std::variant<int, std::string, bool> get(const std::vector<std::variant<int, std::string, bool>>& list, int index) {
    if (index < 0) {
        index = list.size() + index;
    }
    if (index < 0 || index >= list.size()) {
        return std::string("undefined");
    }
    return list[index];
}

std::variant<int, std::string, bool> get(const std::unordered_map<std::string, std::variant<int, std::string, bool>>& map, const std::string& key) {
    auto it = map.find(key);
    if (it != map.end()) {
        return it->second;
    }
    return std::string("undefined");
}

std::variant<int, std::string, bool> get(const std::unordered_map<std::string, std::variant<int, std::string, bool>>& map, const std::string& key, const std::string& defaultValue) {
    auto it = map.find(key);
    if (it != map.end()) {
        return it->second;
    }
    return defaultValue;
}

std::vector<std::string> mapKeys(const std::unordered_map<std::string, std::variant<int, std::string, bool>>& map) {
    std::vector<std::string> keys;
    for (const auto& pair : map) {
        keys.push_back(pair.first);
    }
    return keys;
}

std::vector<std::variant<int, std::string, bool>> mapValues(const std::unordered_map<std::string, std::variant<int, std::string, bool>>& map) {
    std::vector<std::variant<int, std::string, bool>> values;
    for (const auto& pair : map) {
        values.push_back(pair.second);
    }
    return values;
}

// Range generator implementation
class RangeGenerator {
public:
    RangeGenerator(int start, int end, int step = 1)
        : start_(start), end_(end), step_(step) {}
    
    // Iterator class
    class Iterator {
    public:
        Iterator(int value, int step, int end) : value_(value), step_(step), end_(end) {}
        
        int operator*() const { return value_; }
        
        Iterator& operator++() {
            value_ += step_;
            return *this;
        }
        
        bool operator!=(const Iterator& other) const {
            if (step_ > 0) {
                return value_ < other.value_;
            } else {
                return value_ > other.value_;
            }
        }
        
    private:
        int value_;
        int step_;
        int end_;
    };
    
    Iterator begin() const { return Iterator(start_, step_, end_); }
    Iterator end() const { return Iterator(end_, step_, end_); }
    
private:
    int start_;
    int end_;
    int step_;
};

// Range function overloads
RangeGenerator range(int start, int end, int step = 1) {
    return RangeGenerator(start, end, step);
}

RangeGenerator range(double start, double end, double step = 1.0) {
    return RangeGenerator(static_cast<int>(start), static_cast<int>(end), static_cast<int>(step));
}

RangeGenerator range(int end) {
    return RangeGenerator(0, end, 1);
}

RangeGenerator range(double end) {
    return RangeGenerator(0, static_cast<int>(end), 1);
}

// Vanction namespace for utility functions
namespace vanction {
    RangeGenerator range(int start, int end, int step = 1) {
        return ::range(start, end, step);
    }
    
    RangeGenerator range(double start, double end, double step = 1.0) {
        return ::range(start, end, step);
    }
    
    RangeGenerator range(int end) {
        return ::range(end);
    }
    
    RangeGenerator range(double end) {
        return ::range(end);
    }
}

int main() {
    // 测试整数switch
    auto num = 2;
    std::cout << "Testing integer switch with value: " << num << std::endl;
    if (num == 1) {
    std::cout << "Case 1 executed" << std::endl;
    } else if (num == 2) {
    std::cout << "Case 2 executed" << std::endl;
    } else if (num == 3) {
    std::cout << "Case 3 executed" << std::endl;
    }
    // 测试字符串switch
    auto str = "hello";
    std::cout << "\nTesting string switch with value: " << str << std::endl;
    if (str == "hello") {
    std::cout << "Hello case executed" << std::endl;
    } else if (str == "world") {
    std::cout << "World case executed" << std::endl;
    }
    // 测试布尔值switch
    auto flag = true;
    std::cout << "\nTesting boolean switch with value: " << flag << std::endl;
    if (flag == true) {
    std::cout << "True case executed" << std::endl;
    } else if (flag == false) {
    std::cout << "False case executed" << std::endl;
    }
    return 0;
}

