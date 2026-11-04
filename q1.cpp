
#include <bits/stdc++.h>
using namespace std;

struct Rule {
    string lhs;
    vector<string> rhs;
};

struct Node {
    string val;
    vector<Node*> child;
    Node(string s) : val(s) {}
};

vector<Rule> rules;
set<string> nt;
vector<string> target;

vector<string> split(string s) {
    stringstream ss(s);
    vector<string> v;
    string x;
    while (ss >> x) v.push_back(x);
    return v;
}

string join(vector<string> v) {
    string s;
    for (string x : v)
        s += (s.empty() ? "" : " ") + x;
    return s.empty() ? "epsilon" : s;
}

string treeText(Node* p) {
    if (p->child.empty()) return p->val;
    string s = p->val + "(";
    for (int i = 0; i < (int)p->child.size(); i++)
        s += (i ? " " : "") + treeText(p->child[i]);
    return s + ")";
}

void printTree(Node* p, string prefix = "",
               bool last = true, bool root = true) {
    cout << prefix
         << (root ? "" : last ? "`-- " : "|-- ")
         << p->val << '\n';

    prefix += root ? "" : last ? "    " : "|   ";

    for (int i = 0; i < (int)p->child.size(); i++)
        printTree(p->child[i], prefix,
                  i == (int)p->child.size()-1, false);
}

Node* leftmost(Node* p) {
    if (p->child.empty())
        return nt.count(p->val) ? p : nullptr;

    for (Node* c : p->child)
        if (Node* x = leftmost(c)) return x;
    return nullptr;
}

bool possible(vector<string> v) {
    int count = 0, i = 0;

    for (string x : v)
        if (!nt.count(x)) count++;

    if (count > (int)target.size()) return false;

    for (string x : v) {
        if (nt.count(x)) break;
        if (i >= (int)target.size() ||
            x != target[i++]) return false;
    }
    return true;
}

string key(vector<string> v) {
    string s;
    for (string x : v)
        s += to_string(x.size()) + ":" + x;
    return s;
}

int main() {
    int n;
    cout << "Number of production lines: ";
    if (!(cin >> n) || n <= 0) return 0;
    cin.ignore();

    string line, start;
    cout << "Enter grammar (spaces between symbols; # for epsilon):\n";

    for (int i = 0; i < n; i++) {
        getline(cin, line);
        int pos = line.find("->");

        if (pos < 0 ||
            split(line.substr(0, pos)).size() != 1) {
            cout << "Invalid grammar\n";
            return 0;
        }

        string lhs = split(line.substr(0, pos))[0];
        if (start.empty()) start = lhs;
        nt.insert(lhs);

        string rhs = line.substr(pos + 2), part;

        if (split(rhs).empty() ||
            rhs[rhs.find_last_not_of(" \t")] == '|') {
            cout << "Invalid grammar\n";
            return 0;
        }

        stringstream ss(rhs);

        while (getline(ss, part, '|')) {
            vector<string> v = split(part);

            if (v.empty() ||
                (find(v.begin(), v.end(), "#") != v.end()
                 && v != vector<string>{"#"})) {
                cout << "Invalid grammar\n";
                return 0;
            }

            if (v == vector<string>{"#"}) v.clear();
            rules.push_back({lhs, v});
        }
    }

    cout << "Input tokens (space-separated; # for empty): ";
    getline(cin, line);
    target = split(line);

    if (target == vector<string>{"#"})
        target.clear();

    queue<pair<vector<string>, vector<int>>> q;
    q.push({{start}, {}});

    set<string> seen = {key({start})};
    vector<int> answer;
    bool found = false;
    int explored = 0, limit = 10000;
    bool truncated = false;

    while (!q.empty() && explored < limit) {
        explored++;
        auto [form, path] = q.front();
        q.pop();

        int at = 0;
        while (at < (int)form.size() &&
               !nt.count(form[at])) at++;

        if (at == (int)form.size()) {
            if (form == target) {
                answer = path;
                found = true;
                break;
            }
            continue;
        }

        for (int r = 0; r < (int)rules.size(); r++) {
            if (rules[r].lhs != form[at]) continue;

            vector<string> next = form;
            next.erase(next.begin() + at);
            next.insert(next.begin() + at,
                        rules[r].rhs.begin(),
                        rules[r].rhs.end());

            if ((int)next.size() >
                max(30, (int)target.size()*4 + 10)) {
                truncated = true;
                continue;
            }

            if (possible(next) &&
                seen.insert(key(next)).second) {
                auto p = path;
                p.push_back(r);
                q.push({next, p});
            }
        }
    }

    if (!found) {
        cout << ((!q.empty() || truncated) ?
                 "Search limit exceeded\n" :
                 "String Rejected\n");
        return 0;
    }

    Node* root = new Node(start);
    vector<string> form = {start};

    cout << "\nStep 0: " << start
         << "\nTree: " << treeText(root) << "\n";

    for (int i = 0; i < (int)answer.size(); i++) {
        Rule r = rules[answer[i]];

        int at = 0;
        while (!nt.count(form[at])) at++;

        form.erase(form.begin() + at);
        form.insert(form.begin() + at,
                    r.rhs.begin(), r.rhs.end());

        Node* p = leftmost(root);

        if (r.rhs.empty())
            p->child.push_back(new Node("epsilon"));
        else
            for (string x : r.rhs)
                p->child.push_back(new Node(x));

        cout << "\nStep " << i + 1 << ": "
             << join(form)
             << "   [" << r.lhs << " -> "
             << join(r.rhs) << "]\n"
             << "Tree: " << treeText(root) << '\n';
    }

    cout << "\nString Accepted\nFinal Parse Tree:\n";
    printTree(root);
    cout << "Derivation steps: "
         << answer.size() << '\n';
}
