#include <bits/stdc++.h>
using namespace std;

struct Instruction {
    string op, a, b, result;
    int leftRef, rightRef;
};

bool isOperator(const string& s) {
    return s == "+" || s == "-" || s == "*" || s == "/";
}
int precedence(const string& s) {
    return (s == "*" || s == "/") ? 2 : 1;
}
bool operand(const string& s) {
    return !s.empty() && (isalnum((unsigned char)s[0]) || s[0] == '_');
}

vector<string> tokenize(const string& s) {
    vector<string> t;
    for (int i = 0; i < (int)s.size();) {
        if (isspace((unsigned char)s[i])) { ++i; continue; }
        if (isalnum((unsigned char)s[i]) || s[i] == '_') {
            int j = i;
            while (j < (int)s.size() && (isalnum((unsigned char)s[j]) || s[j] == '_')) ++j;
            t.push_back(s.substr(i, j-i)); i=j;
        } else if (string("+-*/()").find(s[i]) != string::npos) {
            t.push_back(string(1,s[i++]));
        } else throw runtime_error("Invalid character in expression");
    }
    return t;
}

vector<string> toPostfix(const vector<string>& in) {
    vector<string> out, ops;
    bool needOperand = true;
    for (const string& s : in) {
        if (operand(s)) {
            if (!needOperand) throw runtime_error("Missing operator");
            out.push_back(s);
            needOperand = false;
        } else if (s == "(") {
            if (!needOperand) throw runtime_error("Missing operator before (");
            ops.push_back(s);
        } else if (s == ")") {
            if (needOperand) throw runtime_error("Empty parentheses or missing operand");
            while (!ops.empty() && ops.back() != "(") {
                out.push_back(ops.back()); ops.pop_back();
            }
            if (ops.empty()) throw runtime_error("Unmatched )");
            ops.pop_back();
            needOperand = false;
        } else if (isOperator(s)) {
            if (needOperand) throw runtime_error("Missing left operand");
            while (!ops.empty() && ops.back() != "(" && precedence(ops.back()) >= precedence(s)) {
                out.push_back(ops.back()); ops.pop_back();
            }
            ops.push_back(s);
            needOperand = true;
        }
    }
    if (needOperand) throw runtime_error("Expression ends before an operand");
    while (!ops.empty()) {
        if (ops.back() == "(") throw runtime_error("Unmatched (");
        out.push_back(ops.back()); ops.pop_back();
    }
    return out;
}

int main() {
    cout << "Enter expression: ";
    string line; getline(cin, line);
    try {
        vector<string> postfix = toPostfix(tokenize(line));
        cout << "Postfix: ";
        for (const string& x : postfix) cout << x << ' ';
        cout << "\n\nThree Address Code:\n";

        struct Value { string name; int ref; };
        vector<Value> st;
        vector<Instruction> code;
        for (const string& x : postfix) {
            if (!isOperator(x)) {
                st.push_back({x, -1});
            } else {
                if (st.size() < 2) throw runtime_error("Missing operand");
                Value right = st.back(); st.pop_back();
                Value left = st.back(); st.pop_back();
                string temp = "t" + to_string(code.size() + 1);
                cout << temp << " = " << left.name << " " << x << " " << right.name << '\n';
                code.push_back({x,left.name,right.name,temp,left.ref,right.ref});
                st.push_back({temp,(int)code.size()-1});
            }
        }
        if (st.size() != 1) throw runtime_error("Invalid expression");
        cout << "\nQuadruples:\n";
        cout << "Op\tArg1\tArg2\tResult\n";
        for (const auto& c : code)
            cout << c.op << '\t' << c.a << '\t' << c.b << '\t' << c.result << '\n';
        cout << "\nTriples:\n";
        cout << "Index\tOp\tArg1\tArg2\n";
        for (int i = 0; i < (int)code.size(); ++i) {
            const auto& c = code[i];
            string a = c.leftRef == -1 ? c.a : "(" + to_string(c.leftRef) + ")";
            string b = c.rightRef == -1 ? c.b : "(" + to_string(c.rightRef) + ")";
            cout << i << '\t' << c.op << '\t' << a << '\t' << b << '\n';
        }
        if (code.empty()) cout << "(No operations needed)\n";
        cout << "\nFinal result: " << st.back().name << '\n';
    } catch (const exception& e) {
        cout << "Invalid expression: " << e.what() << '\n';
    }
}
