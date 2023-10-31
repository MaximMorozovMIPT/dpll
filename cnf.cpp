#include <iostream>
#include <cassert>
#include <set>
#include <unordered_set>
#include <limits>
#include <list>
#include <algorithm>

#include "cnf.h"
static int16_t clauses_num;

CNF ParseFile(std::ifstream& istr) {
    std::string comment_str;
    while (istr.peek() == 'c') {
        std::getline(istr, comment_str);
    }

    char first_literal;
    std::string cnf_str;
    int16_t variables, clauses;

    istr >> first_literal >> cnf_str >> variables >> clauses;
    clauses_num = clauses;

    assert(first_literal == 'p');
    assert(cnf_str == "cnf");

    CNF cnf;
    cnf.heuristics_deep = 0;

    int max_elems = 0;
    for (int16_t i = 0; i < clauses; ++i) {
        std::vector<int16_t> clause;
        int16_t num = 0;
        int max_elems_row = 0;
        while(1) {
            istr >> num;
            if (!num) {
                break;
            }
            max_elems_row++;
            clause.push_back(num);
            if (cnf.var_to_clauses_.find(num) == cnf.var_to_clauses_.end()) {
                cnf.var_to_clauses_[num] = std::make_shared<std::unordered_set<int16_t>>();
            }
            cnf.var_to_clauses_[num]->insert(i);
        }
        if (max_elems_row > max_elems) {
            max_elems = max_elems_row;
        }
        if (clause.size() == 1) {
            cnf.mono_clauses.push_back(cnf.cnf_.size());
        }
        // std::cout << clause.bucket_count() << std::endl;
        cnf.cnf_.push_back(std::make_shared<std::vector<int16_t>>(clause));
    }
    std::cout << max_elems << std::endl;

    return cnf;
}

bool CNF::DPLL(std::list<CNF> &cnf_list_ref) {
    while(true) {
        while (!mono_clauses.empty()) {
            std::vector<int16_t> mono_clauses_addition;
            // Propagate 1
            for (auto mono_clause: mono_clauses) {
                excluded_clauses.insert(mono_clause);
                int val = *(cnf_[mono_clause]->begin());

                if (var_to_clauses_.find(val) != var_to_clauses_.end()) {
                    auto deleted_vars = *(var_to_clauses_[val]);
                    for (auto j: deleted_vars) {
                        excluded_clauses.insert(j);
                        for (auto vars: *(cnf_[j])) {
                            if (var_to_clauses_[vars].use_count() != 1) {
                                var_to_clauses_[vars] = std::make_shared<std::unordered_set<int16_t>>(*(var_to_clauses_[vars]));
                            }
                            var_to_clauses_[vars]->erase(j);
                        }
                    }
                }

                if (var_to_clauses_.find(-val) != var_to_clauses_.end()) {
                    for (auto j: *(var_to_clauses_[-val])) {
                        if (excluded_clauses.find(j) != excluded_clauses.end()) {
                            continue;
                        }

                        if (cnf_[j]->size() == 1) {
                            return false;
                        }

                        if (cnf_[j].use_count() != 1) {
                            cnf_[j] = std::make_shared<std::vector<int16_t>>(*(cnf_[j]));
                        }

                        cnf_[j]->erase(std::find(cnf_[j]->begin(), cnf_[j]->end(), -val));

                        if (cnf_[j]->size() == 0) {
                            return false;
                        }
                        if (cnf_[j]->size() == 1) {
                            mono_clauses_addition.push_back(j);
                        }
                    }
                }

                var_to_clauses_.erase(val);
                var_to_clauses_.erase(-val);
            }
            mono_clauses.clear();
            mono_clauses = std::move(mono_clauses_addition);
        }
        
        if (var_to_clauses_.empty() || excluded_clauses.size() == cnf_.size()) {
            // for (auto d: desicions) {
            //     std::cout << d << std::endl;
            // }
            return true;
        }

        int16_t var_to_propagate = -1;
        if (clauses_num > 2000) {
            auto min_clause = cnf_.front();
            for (int i = 0; i < cnf_.size(); ++i) {
                if (excluded_clauses.find(i) == excluded_clauses.end()) {
                    min_clause = cnf_[i];
                    break;
                }
            }
            var_to_propagate = *(min_clause->begin());
            var_to_propagate *= -1;

            if (heuristics_deep == 1) {
                var_to_propagate *= -1;
            }
            // if (heuristics_deep == 6 || heuristics_deep == 8) {
            //     var_to_propagate *= -1;
            // }
        } else {
            int val_max = var_to_clauses_.begin()->first;
            int size_max = var_to_clauses_.begin()->second->size();
            for (auto [val, clause]: var_to_clauses_) {
                if (clause->size() > size_max) {
                    val_max = val;
                    size_max = clause->size();
                }
            }
            var_to_propagate = val_max;  
        }
        

        bool insert_positive = true;

        CNF positive;
        positive.cnf_ = cnf_;
        positive.excluded_clauses = excluded_clauses;
        positive.var_to_clauses_ = var_to_clauses_;
        positive.mono_clauses = std::vector<int16_t>();
        // positive.desicions = desicions;

        // positive.desicions.push_back(-1);
        positive.heuristics_deep = heuristics_deep + 1;
        if (positive.var_to_clauses_.find(-var_to_propagate) != positive.var_to_clauses_.end()) {
            auto deleted_vars = *(positive.var_to_clauses_[-var_to_propagate]);
            for (auto j: deleted_vars) {
                positive.excluded_clauses.insert(j);
                for (auto vars: *(positive.cnf_[j])) {
                    if (-var_to_propagate == vars) {
                        continue;
                    }
                    if (positive.var_to_clauses_[vars].use_count() != 1) {
                        positive.var_to_clauses_[vars] = std::make_shared<std::unordered_set<int16_t>>(*(positive.var_to_clauses_[vars]));
                    }
                    positive.var_to_clauses_[vars]->erase(j);
                }
            }
        }


        if (positive.var_to_clauses_.find(var_to_propagate) != positive.var_to_clauses_.end()) {
            for (auto j: *(positive.var_to_clauses_[var_to_propagate])) {
                if (positive.cnf_[j]->size() == 1) {
                    insert_positive = false;
                    break;
                }
                if (positive.cnf_[j].use_count() != 1) {
                    positive.cnf_[j] = std::make_shared<std::vector<int16_t>>(*(positive.cnf_[j]));
                }
                positive.cnf_[j]->erase(std::find(positive.cnf_[j]->begin(), positive.cnf_[j]->end(), var_to_propagate));
                if (positive.cnf_[j]->size() == 1) {
                    positive.mono_clauses.push_back(j);
                }
                if (positive.cnf_[j]->size() == 0) {
                    insert_positive = false;
                    break;
                }
            }
        }

        positive.var_to_clauses_.erase(var_to_propagate);
        positive.var_to_clauses_.erase(-var_to_propagate);
        
        if (insert_positive) {
            cnf_list_ref.push_front(std::move(positive));
        }
        
        // desicions.push_back(1);
        heuristics_deep++;
        if (var_to_clauses_.find(var_to_propagate) != var_to_clauses_.end()) {
            auto deleted_vars2 = *(var_to_clauses_[var_to_propagate]);
            for (auto j: deleted_vars2) {
                excluded_clauses.insert(j);
                for (auto vars: *(cnf_[j])) {
                    if (var_to_propagate == vars) {
                        continue;
                    }
                    if (var_to_clauses_[vars].use_count() != 1) {
                        var_to_clauses_[vars] = std::make_shared<std::unordered_set<int16_t>>(*(var_to_clauses_[vars]));
                    }
                    var_to_clauses_[vars]->erase(j);
                }
            }
        }
        if (var_to_clauses_.find(-var_to_propagate) != var_to_clauses_.end()) {
            for (auto j: *(var_to_clauses_[-var_to_propagate])) {
                if (cnf_[j]->size() == 1) {
                    return false;
                }
                if (cnf_[j].use_count() != 1) {
                    cnf_[j] = std::make_shared<std::vector<int16_t>>(*(cnf_[j]));
                }
                cnf_[j]->erase(std::find(cnf_[j]->begin(), cnf_[j]->end(), -var_to_propagate));
                if (cnf_[j]->size() == 1) {
                    mono_clauses.push_back(j);
                }
                if (cnf_[j]->size() == 0) {
                    return false;
                }
            }
        }

        var_to_clauses_.erase(var_to_propagate);
        var_to_clauses_.erase(-var_to_propagate);
    }

    return false;   
}
