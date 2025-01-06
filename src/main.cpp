// #include "Lexer.hpp"
// #include "Lexical_anal.hpp"
// #include "Tokens.hpp"
// #include <algorithm>
// #include <bits/getopt_core.h>
// #include <concepts>
// #include <cstdlib>
// #include <filesystem>
// #include <getopt.h>
// #include <iostream>
// #include <sstream>
// #include <string>
// #include <variant>
// #include <vector>
// namespace fs = std::filesystem;

// std::vector<std::string> input_files;

// template<typename T>
// concept printable = requires(T arg){
//     {arg} -> std::convertible_to<std::string>;
// };
// // workaround, non-const lvalue reference cannot bind to an r-value/temporary
// // sooooo, no default arguments
// std::string placeholder;
// // can be hacked with another type T1&&, which implements a concept confining it to std::same_as std::string, but not worth it

// template<printable T>
// bool add_input_file(T arg, std::string& description = placeholder){
//     fs::path path{arg};
//     if (path.extension() != ".し++"){
//         description = "Wrong extension, expected '.し++'\n";
//         return false;
//     }
//     if (path.has_root_path() && fs::is_regular_file(path)){
//         std::cout << "Sucess!, added file: " << path << " to the list\n";
//         input_files.emplace_back(path);
//         return true;
//     }
//     else if (fs::is_regular_file(fs::current_path() /= path)){
//         std::cout << "Sucess!, added file: " << path << " to the list\n";
//         input_files.emplace_back(fs::current_path() /= path);
//         return true;
//     }
//     description = std::string{"No such file: "} + arg;
//     return false;
// }

// int main(int argc, char* argv[]){
//     using std::cout;
//     std::ostringstream osrting;
//     cout << "The number of arguments is: " << argc << '\n';
//     std::string error;
//     if (argc == 2){
//         if (!add_input_file(argv[1], error)) {cout << error << '\n'; exit(1);}

//     }

//     static struct option long_options[] = {
//         {"help", no_argument, NULL, 'h'},
//         {"input", required_argument, NULL, 'i'},
//         {0, 0, 0, 0}
//     };
//     int option;
//     while ((option = getopt_long(argc, argv, "hi:", long_options, NULL)) != -1){
//         switch (option) {
//             case 'h':
//                 std::cout << "Welcome to the help menue\n"; break;
//             case 'i':
//                 cout << "encountered this: " << optarg << '\n';
//                 if (!add_input_file(optarg, error)) {std::cout << error << '\n'; exit(1);}
//                 break;
                
//             default:
//                 std::cout << "No option passed exiting\n";
//                 return EXIT_SUCCESS;
//         }
//     }
//     if (optind < argc){
//         cout << "non-option ARGV-elements\n";
//         while (optind < argc)
//             cout << argv[optind++] << '\n';
//         cout << '\n';
//     }
//     // cout << "the input files are: \n";
//     // std::for_each(input_files.begin(), input_files.end(), [](std::string out){cout << out << '\n';});
//     // exit(1);
//     TOKENS return_value;
//     auto printer = [&return_value](auto& T) {return_value = T.token;};
//     if (!input_files.empty()){
//         Lexer lex(input_files[0]);
//         auto scaned = lex.scan();
//         std::visit(printer, *scaned);
//         cout << find_string_repr(return_value);

//     }
// }