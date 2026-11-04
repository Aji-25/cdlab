
#include <bits/stdc++.h>
using namespace std;

using RHS = vector<string>;

map<string,vector<RHS>> grammar;
set<string> nt, terminals;
map<string,set<string>> firstvt, lastvt;
map<pair<string,string>,set<char>> table;

RHS tokenize(string s) {
    RHS v;
    for (int i = 0; i < (int)s.size();) {
        if (isspace((unsigned char)s[i])) {
            i++;
            continue;
        }

        if (isalpha((unsigned char)s[i]) || s[i] == '_') {
            int j = i+1;
            while (j < (int)s.size() &&
                   (isalnum((unsigned char)s[j]) ||
                    s[j] == '_' || s[j] == '\'')) j++;
            v.push_back(s.substr(i,j-i));
            i = j;
        } else {
            v.push_back(string(1,s[i++]));
        }
    }
    return v;
}

bool isNT(string s) {
    return nt.count(s);
}

string show(set<string> s) {
    string out = "{";
    for (string x : s)
        out += (out.size() == 1 ? "" : ", ") + x;
    return out + "}";
}

void put(string a, string b, char rel) {
    table[{a,b}].insert(rel);
}

int main() {
    int n;
    cout << "Number of productions: ";
    if (!(cin >> n) || n <= 0) return 0;
    cin.ignore();

    vector<pair<string,string>> raw;
    vector<string> order;

    cout << "Enter grammar (E->E+T|T):\n";

    for (int i = 0; i < n; i++) {
        string line;
        getline(cin,line);
        auto p = line.find("->");

        if (p == string::npos) {
            cout << "Invalid grammar\n";
            return 0;
        }

        RHS lhs = tokenize(line.substr(0,p));

        if (lhs.size() != 1 || lhs[0].empty() ||
            !isupper((unsigned char)lhs[0][0])) {
            cout << "Invalid LHS\n";
            return 0;
        }

        if (nt.insert(lhs[0]).second)
            order.push_back(lhs[0]);

        raw.push_back({lhs[0],line.substr(p+2)});
    }

    for (auto [A,text] : raw) {
        if (text.find_first_not_of(" \t") == string::npos ||
            text[text.find_last_not_of(" \t")] == '|') {
            cout << "Invalid grammar\n";
            return 0;
        }

        string part;
        stringstream ss(text);

        while (getline(ss,part,'|')) {
            RHS rhs = tokenize(part);

            if (rhs.empty() || rhs == RHS{"#"} ||
                rhs == RHS{"eps"} ||
                rhs == RHS{"epsilon"}) {
                cout << "Epsilon productions not allowed\n";
                return 0;
            }

            for (string x : rhs) {
                if (!isNT(x)) {
                    if (isupper((unsigned char)x[0])) {
                        cout << "Undefined nonterminal " << x << "\n";
                        return 0;
                    }
                    terminals.insert(x);
                }
            }

            for (int j = 1; j < (int)rhs.size(); j++)
                if (isNT(rhs[j-1]) && isNT(rhs[j])) {
                    cout << "Adjacent nonterminals not allowed\n";
                    return 0;
                }

            grammar[A].push_back(rhs);
        }
    }

    bool changed = true;

    while (changed) {
        changed = false;

        for (auto [A,alts] : grammar)
            for (auto rhs : alts) {
                int k = rhs.size();

                if (!isNT(rhs[0]))
                    changed |= firstvt[A].insert(rhs[0]).second;

                if (k > 1 && isNT(rhs[0]) && !isNT(rhs[1]))
                    changed |= firstvt[A].insert(rhs[1]).second;

                if (isNT(rhs[0]))
                    for (string x : firstvt[rhs[0]])
                        changed |= firstvt[A].insert(x).second;

                if (!isNT(rhs[k-1]))
                    changed |= lastvt[A].insert(rhs[k-1]).second;

                if (k > 1 && isNT(rhs[k-1]) && !isNT(rhs[k-2]))
                    changed |= lastvt[A].insert(rhs[k-2]).second;

                if (isNT(rhs[k-1]))
                    for (string x : lastvt[rhs[k-1]])
                        changed |= lastvt[A].insert(x).second;
            }
    }

    cout << "\nFIRSTVT:\n";
    for (string A : order)
        cout << A << " = " << show(firstvt[A]) << "\n";

    cout << "LASTVT:\n";
    for (string A : order)
        cout << A << " = " << show(lastvt[A]) << "\n";

    for (auto [A,alts] : grammar)
        for (auto rhs : alts) {
            int k = rhs.size();

            for (int i = 0; i+1 < k; i++) {
                string x = rhs[i], y = rhs[i+1];

                if (!isNT(x) && !isNT(y))
                    put(x,y,'=');

                if (!isNT(x) && isNT(y))
                    for (string z : firstvt[y])
                        put(x,z,'<');

                if (isNT(x) && !isNT(y))
                    for (string z : lastvt[x])
                        put(z,y,'>');

                if (i+2 < k && !isNT(x) &&
                    isNT(y) && !isNT(rhs[i+2]))
                    put(x,rhs[i+2],'=');
            }
        }

    for (string x : firstvt[order[0]])
        put("$",x,'<');

    for (string x : lastvt[order[0]])
        put(x,"$",'>');

    put("$","$",'=');

    cout << "\nPrecedence relations:\n";
    for (auto [p,rels] : table)
        for (char r : rels)
            cout << p.first << " " << r << " "
                 << p.second << "\n";

    vector<string> cols(terminals.begin(),terminals.end());
    cols.push_back("$");

    cout << "\nOperator Precedence Table "
         << "(row=stack, column=input):\n";

    cout << setw(10) << " ";
    for (string x : cols)
        cout << setw(10) << x;
    cout << "\n";

    bool conflict = false;

    for (string a : cols) {
        cout << setw(10) << a;

        for (string b : cols) {
            string cell;
            for (char r : table[{a,b}])
                cell += r;

            if (cell.size() > 1)
                conflict = true;

            cout << setw(10)
                 << (cell.empty() ? "-" : cell);
        }
        cout << "\n";
    }

    cout << (conflict ?
        "Conflict detected: not an operator precedence grammar\n" :
        "No precedence conflicts detected\n");
}
