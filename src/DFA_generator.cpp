#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <set>
#include <map>
#include <random>
#include <algorithm>
#include <ctime>

struct DFA {
    int num_states;
    int alphabet_size;
    int start_state;
    std::set<int> accepting_states;
    // transitions[state][char_index] = next_state
    std::vector<std::vector<int>> transitions;

    DFA(int n, int alpha) : num_states(n), alphabet_size(alpha), start_state(0) {
        transitions.resize(n, std::vector<int>(alpha, 0));
    }
};

int random_int(int min, int max) {
    static std::mt19937 rng(std::time(nullptr));
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

// 1. Random DFA generator
DFA generate_random_dfa(int n, int alpha) {
    DFA dfa(n, alpha);
    
    // Transfer state generator
    for (int i = 0; i < n; ++i) {
        for (int c = 0; c < alpha; ++c) {
            dfa.transitions[i][c] = random_int(0, n - 1);
        }
    }
    
    // Random accpeting states
    for (int i = 0; i < n; ++i) {
        if (random_int(0, 1) == 1) {
            dfa.accepting_states.insert(i);
        }
    }
    
    // Ensure start state is not always finish
    if (dfa.accepting_states.empty()) {
        dfa.accepting_states.insert(n - 1);
    }

    return dfa;
}

// 2. DFA breaker (Save L, but Increase num of states)
// Equivalent State Splitting
void expand_dfa(DFA &dfa, int target_size) {
    static std::mt19937 rng(std::time(nullptr));

    while (dfa.num_states < target_size) {
        int current_n = dfa.num_states;
        
        // Choose a random state to split (call u)
        int u = random_int(0, current_n - 1);
        
        // u' = u
        int u_prime = current_n; 
        dfa.transitions.push_back(dfa.transitions[u]); // Copy transitions outgoing from u
        
        // if u is accepting, u' is also accepting
        if (dfa.accepting_states.count(u)) {
            dfa.accepting_states.insert(u_prime);
        }

        // Increase num state
        dfa.num_states++;

        // Redirect edges to u
        // Loop all edges
        bool redirected_any = false;
        for (int i = 0; i < current_n; ++i) {
            for (int c = 0; c < dfa.alphabet_size; ++c) {
                if (dfa.transitions[i][c] == u) {
                    // Rate 50/50 to handle comphresion
                    if (random_int(0, 1) == 1) {
                        dfa.transitions[i][c] = u_prime;
                        redirected_any = true;
                    }
                }
            }
        }
    }
}

// Export to JSON
void write_single_dfa_json(std::ofstream &file, const DFA &dfa, bool is_last = false) {
    file << "    {\n";
    file << "      \"num_states\": " << dfa.num_states << ",\n";
    file << "      \"alphabet_size\": " << dfa.alphabet_size << ",\n";
    file << "      \"start_state\": " << dfa.start_state << ",\n";
    
    file << "      \"accepting_states\": [";
    int count = 0;
    for (int s : dfa.accepting_states) {
        file << s << (count < dfa.accepting_states.size() - 1 ? ", " : "");
        count++;
    }
    file << "],\n";

    file << "      \"transitions\": [\n";
    for (int i = 0; i < dfa.num_states; ++i) {
        file << "        [";
        for (int c = 0; c < dfa.alphabet_size; ++c) {
            file << dfa.transitions[i][c] << (c < dfa.alphabet_size - 1 ? ", " : "");
        }
        file << "]" << (i < dfa.num_states - 1 ? ",\n" : "\n");
    }
    file << "      ]\n";
    file << "    }" << (is_last ? "" : ",") << "\n";
}

int main() {
    // Config
    int base_states = 5; 
    int alphabet_size = 2;    // Num of char
    
    std::cout << "Dang sinh DFA co so (Minimal)..." << std::endl;
    // 1. Generate minimal DFA
    DFA minimal_dfa = generate_random_dfa(base_states, alphabet_size);

    // Save minimal DFA
    std::ofstream min_file("minimal_dfa.json");
    min_file << "{\n  \"dfa_minimal\": \n";
    write_single_dfa_json(min_file, minimal_dfa, true);
    min_file << "}\n";
    min_file.close();
    std::cout << "-> Da luu: minimal_dfa.json" << std::endl;

    // 2. Split into nonminimal DFA
    std::vector<int> targets = {100, 1000, 5000};
    
    std::ofstream exp_file("expanded_dfas.json");
    exp_file << "{\n  \"expanded_dfas\": [\n";

    for (size_t i = 0; i < targets.size(); ++i) {
        int target = targets[i];
        std::cout << "Dang pha DFA thanh " << target << " states..." << std::endl;
        
        // Copy from original to ensure equivilance
        DFA expanded = minimal_dfa;
        
        // Start to split
        expand_dfa(expanded, target);
        
        // Write into JSON file
        write_single_dfa_json(exp_file, expanded, (i == targets.size() - 1));
    }

    exp_file << "  ]\n}\n";
    exp_file.close();
    std::cout << "-> Da luu: expanded_dfas.json" << std::endl;
    std::cout << "Hoan tat!" << std::endl;

    return 0;
}
