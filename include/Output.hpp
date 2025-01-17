#ifndef OUTPUT_HEADER
#define OUTPUT_HEADER

#include "output.hpp"
#include <concepts>
#include <fstream>
#include <ostream>
#include <variant>
#include <vector>


#include <fstream>
#include <string>
#include <variant>
#include <vector>
#include <iostream>
#include <type_traits>

// using output_types = std::variant<std::ofstream&, std::vector<std::string>&>;

// template<typename T>
// concept allowed_types = std::is_same_v<T, std::ofstream&> || std::is_same_v<T, std::vector<std::string>&>;

// template<allowed_types T>
// class Output_locations {
//     T output_location;
// public:
//     Output_locations(T out_loc) : output_location{ out_loc } {}

//     void emit(const std::string& str) {
//         if constexpr (std::is_same_v<T, std::ofstream&>) {
//             output_location << str;
//         }
//         else {
//             output_location.emplace_back(str);
//         }
//     }
// };
inline std::vector<std::string> defalut;
inline std::ofstream default_file;
struct Output_locations{
    static std::ofstream& out_file;
    public:
        void emit(std::string& str){
            out_file << str;
        }
};

#endif