#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <sstream>

using namespace std;

map<string, vector<vector<string>>> grammar;
vector<string> input;
int curr_pos = 0;

vector<string> split(string s) {
    stringstream ss(s);
    vector<string> v;
    string x;
    while (ss >> x) v.push_back(x);
    return v;
}

bool parse(string symbol) {
    if (grammar.find(symbol) == grammar.end()) {
        if (symbol == "#") return true;
        
        if (curr_pos < input.size() && input[curr_pos] == symbol) {
            curr_pos++;
            return true;
        }
        return false;
    }

    int saved_pos = curr_pos;

    for (auto production : grammar[symbol]) {
        bool production_success = true;
        
        for (string rhs_symbol : production) {
            if (!parse(rhs_symbol)) {
                production_success = false;
                break;
            }
        }
        
        if (production_success) return true;
        
        curr_pos = saved_pos;
    }
    
    return false;
}

int main() {
    int n;
    cout << "Number of production lines: ";
    if (!(cin >> n) || n <= 0) return 0;
    cin.ignore();

    cout << "Enter grammar (e.g., S -> a S b | c | #):\n";
    string start_symbol = "";
    
    for (int i = 0; i < n; ++i) {
        string line;
        getline(cin, line);
        
        size_t arrow_pos = line.find("->");
        string lhs = split(line.substr(0, arrow_pos))[0];
        string rhs_full = line.substr(arrow_pos + 2);
        
        if (start_symbol == "") start_symbol = lhs;
        
        stringstream ss(rhs_full);
        string part;
        while (getline(ss, part, '|')) {
            grammar[lhs].push_back(split(part));
        }
    }

    cout << "Input tokens (space-separated): ";
    string line;
    getline(cin, line);
    input = split(line);

    curr_pos = 0;
    bool accepted = parse(start_symbol);

    if (accepted && curr_pos == input.size()) {
        cout << "String Accepted\n";
    } else {
        cout << "String Rejected\n";
    }

    return 0;
}
