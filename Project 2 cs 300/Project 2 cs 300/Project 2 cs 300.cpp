// Project 2 cs 300.cpp : This file contains the 'main' function. Program execution begins and ends there.
// Jasmine Villarreal

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>
#include <unordered_set>

using namespace std;

// ---------------------- Models ----------------------
struct Course {
    string id;                 // e.g., CS200
    string title;              // e.g., Data Structures
    vector<string> prereqIds;  // e.g., {"CS100","MATH200"}
};

static string trim(const string& s) {
    size_t b = s.find_first_not_of(" \t\r\n");
    if (b == string::npos) return "";
    size_t e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

static string toUpper(string s) {
    for (char& c : s) c = static_cast<char>(toupper(static_cast<unsigned char>(c)));
    return s;
}

// ---------------------- Hash Table (chaining) ----------------------
class HashTable {
public:
    explicit HashTable(size_t capacity = 179) : cap_(capacity), buckets_(capacity) {}

    void insert(const Course& c) {
        size_t idx = hashKey(c.id);
        // replace if exists
        for (auto& existing : buckets_[idx]) {
            if (existing.id == c.id) { existing = c; return; }
        }
        buckets_[idx].push_back(c);
    }

    bool find(const string& courseId, Course& out) const {
        size_t idx = hashKey(courseId);
        for (const auto& c : buckets_[idx]) {
            if (c.id == courseId) { out = c; return true; }
        }
        return false;
    }

    vector<Course> toList() const {
        vector<Course> all;
        for (const auto& chain : buckets_) {
            for (const auto& c : chain) {
                all.push_back(c);
            }
        }
        return all;
    }

private:
    size_t cap_;
    vector<vector<Course>> buckets_;

    size_t hashKey(const string& key) const {
        // simple polynomial rolling hash
        long long h = 0;
        for (unsigned char ch : key) {
            h = (h * 31 + static_cast<int>(ch)) % static_cast<long long>(cap_);
        }
        if (h < 0) h += static_cast<long long>(cap_);
        return static_cast<size_t>(h);
    }
};

bool validateAllPrereqs(const HashTable& table) {
    // Build a fast lookup of all course IDs
    const auto all = table.toList();
    unordered_set<string> ids;
    ids.reserve(all.size() * 2);
    for (const auto& c : all) ids.insert(c.id);

    bool ok = true;

    // Check every course's prereqs
    for (const auto& c : all) {
        for (const auto& p : c.prereqIds) {
            if (p == c.id) {
                cout << "Warning: " << c.id << " lists itself as a prerequisite.\n";
                ok = false;
            }
            else if (!ids.count(p)) {
                cout << "Warning: prerequisite " << p
                    << " referenced by " << c.id
                    << " was not found in the dataset.\n";
                ok = false;
            }
        }
    }
    return ok;
}


// ---------------------- Globals ----------------------
HashTable gTable(179); // prime capacity
bool gDataLoaded = false;

// ---------------------- CSV Loader ----------------------
void loadDataFromFile(const string& path) {
    if (path.empty()) {
        cout << "Error: file path is required.\n";
        return;
    }

    ifstream in(path);
    if (!in) {
        cout << "Error: could not open file '" << path << "'.\n";
        return;
    }

    string line;
    size_t lineNo = 0;
    size_t loaded = 0;

    while (getline(in, line)) {
        ++lineNo;
        line = trim(line);
        if (line.empty()) continue;

        // Parse CSV: id,title,pr1,pr2,...
        vector<string> tokens;
        {
            string field;
            stringstream ss(line);
            while (getline(ss, field, ',')) {
                tokens.push_back(trim(field));
            }
        }

        if (tokens.size() < 2) {
            cout << "Warning: malformed line " << lineNo
                << " (need at least id and title). Skipped.\n";
            continue;
        }

        Course c;
        c.id = toUpper(tokens[0]);
        c.title = tokens[1];
        c.prereqIds.clear();
        for (size_t i = 2; i < tokens.size(); ++i) {
            if (!tokens[i].empty()) c.prereqIds.push_back(toUpper(tokens[i]));
        }

        // insert the course into the table
        gTable.insert(c);
        ++loaded;
    } // <-- this closes the while loop

    gDataLoaded = true;
    cout << "Load complete. Courses loaded: " << loaded << "\n";

    // Full prerequisite validation sweep
    bool ok = validateAllPrereqs(gTable);
    cout << "Prerequisite validation: "
        << (ok ? "OK" : "issues found (see warnings above)") << "\n";
}



// ---------------------- Output Helpers ----------------------
void printAllCoursesSorted() {
    if (!gDataLoaded) { cout << "Please load data first (Option 1).\n"; return; }

    vector<Course> courses = gTable.toList();
    sort(courses.begin(), courses.end(),
        [](const Course& a, const Course& b) {
            // alphanumeric by ID (case-insensitive, but we store uppercase)
            return a.id < b.id;
        });

    cout << "All Computer Science courses (A–Z):\n";
    for (const auto& c : courses) {
        cout << c.id << ", " << c.title << "\n";
    }
}

void printSingleCourse() {
    if (!gDataLoaded) { cout << "Please load data first (Option 1).\n"; return; }

    cout << "Enter course ID (e.g., CS200): ";
    string id; getline(cin, id);
    id = toUpper(trim(id));

    if (id.empty()) { cout << "No course ID entered.\n"; return; }

    Course c;
    if (!gTable.find(id, c)) {
        cout << "Course " << id << " not found.\n";
        return;
    }

    cout << c.id << ", " << c.title << "\n";
    if (c.prereqIds.empty()) {
        cout << "Prerequisites: None\n";
    }
    else {
        cout << "Prerequisites: ";
        for (size_t i = 0; i < c.prereqIds.size(); ++i) {
            cout << c.prereqIds[i];
            if (i + 1 < c.prereqIds.size()) cout << ", ";
        }
        cout << "\n";
    }
}

// ---------------------- Menu ----------------------
void showMenu() {
    cout << "\n";
    cout << "  1. Load data from file\n";
    cout << "  2. Print course list (A Through Z)\n";
    cout << "  3. Print course info\n";
    cout << "  9. Exit\n";
    cout << "Select an option: ";
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "Welcome to the CS Advising Assistance Program\n";

    while (true) {
        showMenu();
        string choice;
        if (!getline(cin, choice)) break;

        choice = trim(choice);
        if (choice == "1") {
            cout << "Enter CSV file path: ";
            string path; getline(cin, path);
            loadDataFromFile(trim(path));
        }
        else if (choice == "2") {
            printAllCoursesSorted();
        }
        else if (choice == "3") {
            printSingleCourse();
        }
        else if (choice == "9") {
            cout << "Good bye.\n";
            return 0;
        }
        else {
            cout << "Invalid option. Please choose 1, 2, 3, or 9.\n";
        }
    }

    cout << "Good bye.\n";
    return 0;
}
 