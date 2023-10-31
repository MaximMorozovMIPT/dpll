#include <fstream>
#include <iostream>
#include <list>

#include "cnf.h"

int main(int argc, char *argv[]) {
    std::string input_file_name(argv[1]);
    std::ifstream istr(input_file_name);
    auto cnf = ParseFile(istr);

    std::list<CNF> cnf_list;
    cnf_list.push_back(std::move(cnf));
    // std::cout << input_file_name;
    while (!cnf_list.empty()) {
        auto it = cnf_list.begin();
        if (cnf_list.front().DPLL(cnf_list)) {
            std::cout << "SAT" << std::endl;
            return 0;
        }
        cnf_list.erase(it);
    }
    std::cout << "UNSAT" << std::endl;
    return 0;
}
