
#include <bits/stdc++.h>
using namespace std;

struct Rule { char lhs; string rhs; };
using Item = pair<int,int>;
using State = set<Item>;

vector<Rule> rules;
vector<State> states;
vector<char> nts, ts;
map<pair<int,char>,int> edges, gotos;
map<pair<int,char>,string> act;
bool conflict = false;

bool isNT(char c) { return c >= 'A' && c <= 'Z'; }

State closure(State s) {
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto [r,d] : State(s)) {
            if (d == (int)rules[r].rhs.size()) continue;
            char x = rules[r].rhs[d];
            if (!isNT(x)) continue;
            for (int i = 0; i < (int)rules.size(); i++)
                if (rules[i].lhs == x)
                    changed |= s.insert({i,0}).second;
        }
    }
    return s;
}

State go(State s, char x) {
    State ans;
    for (auto [r,d] : s)
        if (d < (int)rules[r].rhs.size() && rules[r].rhs[d] == x)
            ans.insert({r,d+1});
    return ans.empty() ? ans : closure(ans);
}

void put(int i, char x, string a) {
    auto k = make_pair(i,x);
    if (act.count(k) && act[k] != a) {
        conflict = true;
        cout << "Conflict at state " << i << ", symbol " << x
             << ": " << act[k] << " / " << a << "\n";
    } else act[k] = a;
}

void parse(string input) {
    input += '$';
    vector<int> st = {0};
    vector<char> sy;
    int pos = 0;

    cout << "\nStack\tInput\tAction\n";

    for (int step = 0; step < 10000; step++) {
        int state = st.back();
        char x = input[pos];
        string a = act.count({state,x}) ? act[{state,x}] : "error";

        for (int i = 0; i < (int)sy.size(); i++)
            cout << st[i] << sy[i];

        cout << st.back() << "\t" << input.substr(pos) << "\t" << a << "\n";

        if (a == "acc") {
            cout << "String Accepted\n";
            return;
        }
        if (a == "error") {
            cout << "String Rejected\n";
            return;
        }

        if (a[0] == 's') {
            sy.push_back(x);
            st.push_back(stoi(a.substr(1)));
            pos++;
        } else {
            int r = stoi(a.substr(1));
            int len = rules[r].rhs.size();

            if (len > (int)sy.size()) {
                cout << "String Rejected\n";
                return;
            }

            while (len--) {
                sy.pop_back();
                st.pop_back();
            }

            char A = rules[r].lhs;

            if (!gotos.count({st.back(),A})) {
                cout << "String Rejected\n";
                return;
            }

            int next = gotos[{st.back(),A}];
            sy.push_back(A);
            st.push_back(next);
        }
    }
    cout << "Parsing limit exceeded\n";
}

int main() {
    int n;
    cout << "Number of grammar lines: ";
    if (!(cin >> n) || n <= 0) return 0;
    cin.ignore();

    vector<Rule> original;
    set<char> defined, terminalSet;

    cout << "Enter grammar (S->CC, C->cC|d, ~ for epsilon):\n";

    for (int i = 0; i < n; i++) {
        string line;
        getline(cin,line);

        line.erase(remove_if(line.begin(),line.end(),::isspace),
                   line.end());

        if (line.size() < 4 || !isNT(line[0]) ||
            line.substr(1,2) != "->" || line.back() == '|') {
            cout << "Invalid grammar\n";
            return 0;
        }

        char A = line[0];

        if (defined.insert(A).second)
            nts.push_back(A);

        string part;
        stringstream ss(line.substr(3));

        while (getline(ss,part,'|')) {
            if (part.empty()) {
                cout << "Invalid grammar\n";
                return 0;
            }

            if (part == "~") part = "";
            else if (part.find_first_of("~#$") != string::npos) {
                cout << "Invalid grammar symbol\n";
                return 0;
            }

            original.push_back({A,part});
        }
    }

    for (auto r : original)
        for (char x : r.rhs) {
            if (isNT(x) && !defined.count(x)) {
                cout << "Undefined nonterminal " << x << "\n";
                return 0;
            }
            if (!isNT(x) && terminalSet.insert(x).second)
                ts.push_back(x);
        }

    rules.push_back({'#',string(1,nts[0])});
    for (auto r : original) rules.push_back(r);

    cout << "\nAugmented Grammar:\n";
    for (int i = 0; i < (int)rules.size(); i++)
        cout << i << ": " << rules[i].lhs << " -> "
             << (rules[i].rhs.empty() ? "epsilon" : rules[i].rhs)
             << "\n";

    vector<char> symbols = nts;
    symbols.insert(symbols.end(),ts.begin(),ts.end());
    states.push_back(closure({{0,0}}));

    for (int i = 0; i < (int)states.size(); i++)
        for (char x : symbols) {
            State s = go(states[i],x);
            if (s.empty()) continue;

            int j = 0;
            while (j < (int)states.size() && states[j] != s) j++;
            if (j == (int)states.size()) states.push_back(s);
            edges[{i,x}] = j;
        }

    cout << "\nLR(0) Items and GOTO:\n";
    for (int i = 0; i < (int)states.size(); i++) {
        cout << "I" << i << ":\n";

        for (auto [r,d] : states[i]) {
            cout << "  " << rules[r].lhs << " -> ";
            for (int k = 0; k <= (int)rules[r].rhs.size(); k++) {
                if (k == d) cout << '.';
                if (k < (int)rules[r].rhs.size())
                    cout << rules[r].rhs[k];
            }
            cout << "\n";
        }

        for (char x : symbols)
            if (edges.count({i,x}))
                cout << "  GOTO(I" << i << "," << x
                     << ") = I" << edges[{i,x}] << "\n";
    }

    for (auto [k,j] : edges) {
        auto [i,x] = k;
        if (isNT(x)) gotos[k] = j;
        else put(i,x,"s"+to_string(j));
    }

    for (int i = 0; i < (int)states.size(); i++)
        for (auto [r,d] : states[i]) {
            if (d != (int)rules[r].rhs.size()) continue;

            if (r == 0) put(i,'$',"acc");
            else {
                for (char x : ts)
                    put(i,x,"r"+to_string(r));
                put(i,'$',"r"+to_string(r));
            }
        }

    cout << "\nACTION/GOTO Table:\n";
    cout << setw(8) << "State";
    for (char x : ts) cout << setw(8) << x;
    cout << setw(8) << '$';
    for (char x : nts) cout << setw(8) << x;
    cout << "\n";

    for (int i = 0; i < (int)states.size(); i++) {
        cout << setw(8) << i;

        for (char x : ts)
            cout << setw(8)
                 << (act.count({i,x}) ? act[{i,x}] : "-");

        cout << setw(8)
             << (act.count({i,'$'}) ? act[{i,'$'}] : "-");

        for (char x : nts)
            cout << setw(8)
                 << (gotos.count({i,x}) ? to_string(gotos[{i,x}]) : "-");
        cout << "\n";
    }

    if (conflict) {
        cout << "Grammar is NOT LR(0)\n";
        return 0;
    }

    cout << "Grammar is LR(0)\n";
    cout << "Input string (~ for empty): ";

    string input;
    cin >> input;
    if (input == "~") input.clear();

    for (char x : input)
        if (!terminalSet.count(x)) {
            cout << "String Rejected\n";
            return 0;
        }

    parse(input);
}
