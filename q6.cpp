#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <queue>
using namespace std;

int n, m, t, start, f_count;

vector<map<char, set<int>>> nfa;
vector<map<char, set<int>>> dfa_trans; 

vector<char> alph;

set<int> nfa_finals;
set<int> dfa_finals;

string show(set<int> s) {
    if (s.empty()) return "{}";
    string r = "{";
    for (int x : s) r += "q" + to_string(x) + ",";
    r.back() = '}';
    return r;
}


set<int> e_closure(set<int> states) {
    queue<int> q;
    set<int> res = states;
    for (int x : states) q.push(x);
    
    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (int v : nfa[u]['e']) {
            if (res.insert(v).second) q.push(v);
        }
    }
    return res;
}

set<int> move_to(set<int> states, char c) {
    set<int> res;
    for (int u : states)
        for (int v : nfa[u][c]) res.insert(v);
    return res;
}

int main() {
    cout << "Enter states, symbols, transitions count: ";
    cin >> n >> m >> t;
    nfa.resize(n);
    alph.resize(m);
    
    cout << "Enter symbols: ";
    for (int i = 0; i < m; i++) cin >> alph[i];
    
    cout << "Enter transitions (From Char To): \n";
    for (int i = 0; i < t; i++) {
        int u, v; char c; cin >> u >> c >> v;
        nfa[u][c].insert(v);
    }
    
    cout << "Enter start state: "; cin >> start;
    cout << "Enter final states count, then the states: ";
    cin >> f_count;
    for (int i = 0; i < f_count; i++) { int f; cin >> f; nfa_finals.insert(f); }

    cout << "\nStep 1: E-closures\n";
    for (int i = 0; i < n; i++)
        cout << "e-closure(q" << i << ") = " << show(e_closure({i})) << "\n";

    vector<set<int>> dfa_states;
    map<set<int>, int> dfa_id;
    queue<set<int>> q;

    set<int> init = e_closure({start});
    dfa_states.push_back(init);
    dfa_id[init] = 0;
    
    dfa_trans.push_back(map<char, set<int>>()); 
    q.push(init);

    cout << "\nStep 2 & 3: Subset Construction\n";
    while (!q.empty()) {
        auto curr = q.front(); q.pop();
        int u = dfa_id[curr];
        cout << "\nProcessing D" << u << " = " << show(curr) << "\n";

        for (char c : alph) {
            auto m_set = move_to(curr, c);
            auto clo = e_closure(m_set);
            
            cout << "  MOVE(" << show(curr) << ", " << c << ") = " << show(m_set) << "\n";
            cout << "  e-closure(" << show(m_set) << ") = " << show(clo) << "\n";

            if (dfa_id.find(clo) == dfa_id.end()) {
                dfa_id[clo] = dfa_states.size();
                dfa_states.push_back(clo);

                dfa_trans.push_back(map<char, set<int>>()); 
                
                q.push(clo);
                cout << "  => New State D" << dfa_id[clo] << " = " << show(clo) << "\n";
            }
            
            dfa_trans[u][c].insert(dfa_id[clo]);
        }
    }

    cout << "\nStep 4: Final DFA\nState\tSubset";
    for (char c : alph) cout << "\t" << c;
    cout << "\n";

    for (int i = 0; i < dfa_states.size(); i++) {
        bool is_f = false;
        for (int s : dfa_states[i]) {
            if (nfa_finals.count(s)) {
                is_f = true;
                dfa_finals.insert(i);
            }
        }
        
        cout << (i == 0 ? "->" : "  ") << (is_f ? "*" : " ") 
             << "D" << i << "\t" << show(dfa_states[i]);
             
        for (char c : alph) {
            cout << "\tD" << *dfa_trans[i][c].begin();
        }
        cout << "\n";
    }
}