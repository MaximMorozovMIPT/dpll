#include <fstream>
#include <vector>
#include <set>
#include <unordered_set>
#include <list>
#include <unordered_map>
#include <memory>


class CNF {
public:
    bool DPLL(std::list<CNF> &cnf_list_ref);
public:
    int heuristics_deep;
    // std::vector<int> desicions;
    std::vector<std::shared_ptr<std::vector<int16_t>>> cnf_;
    std::unordered_map<int16_t, std::shared_ptr<std::unordered_set<int16_t>>> var_to_clauses_;
    std::unordered_set<int16_t> excluded_clauses;
    std::vector<int16_t> mono_clauses;

};

CNF ParseFile(std::ifstream& istr);
