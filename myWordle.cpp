#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <unordered_set>

using namespace std;

string to_lower_str(string s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return tolower(c); });
    return s;
}
string trim(const string& s) {
    size_t a = 0;
    while (a < s.size() && isspace((unsigned char)s[a])) ++a;
    size_t b = s.size();
    while (b > a && isspace((unsigned char)s[b - 1])) --b;
    return s.substr(a, b - a);
}

//Load words from file
vector<string> load_words(const string& filename) {
    ifstream in(filename);
    vector<string> words;
    if (!in) return words;
    string line;
    while (getline(in, line)) {
        line = trim(line);
        if (line.size() != 5) continue;
        bool ok = true;
        for (char c : line) if (!isalpha((unsigned char)c)) { ok = false; break; }
        if (!ok) continue;
        words.push_back(to_lower_str(line));
    }
    return words;
}

//pick a random secret
string pick_random(const vector<string>& words) {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<size_t> dist(0, words.size() - 1);
    return words[dist(gen)];
}

//compute feedback (Wordle rules for repeated letters handled)
vector<char> get_feedback(const string& secret, const string& guess) {
    //'G' = green, 'Y' = yellow, 'B' = black
    vector<char> res(5, 'B');
    vector<bool> used(5, false);

    //mark greens and mark those positions used
    for (int i = 0; i < 5; ++i) {
        if (guess[i] == secret[i]) {
            res[i] = 'G';
            used[i] = true;
        }
    }
    //count remaining letters in secret
    int counts[26] = { 0 };
    for (int i = 0; i < 5; ++i)
        if (!used[i]) counts[secret[i] - 'a']++;

    //mark yellows for the non-green letters if the secret still has that letter available
    for (int i = 0; i < 5; ++i) {
        if (res[i] == 'G') continue;
        int idx = guess[i] - 'a';
        if (counts[idx] > 0) {
            res[i] = 'Y';
            counts[idx]--;
        }
    }
    return res;
}

//ANSI color printing
void print_feedback(const string& guess, const vector<char>& fb) {
    const string GREEN = "\033[1;32m";   //bright green
    const string YELLOW = "\033[1;33m";  //bright yellow
    const string GRAY = "\033[1;90m";    //gray
    const string RESET = "\033[0m";
    for (int i = 0; i < 5; ++i) {
        char C = toupper(guess[i]);
        if (fb[i] == 'G') cout << GREEN << C << RESET;
        else if (fb[i] == 'Y') cout << YELLOW << C << RESET;
        else cout << GRAY << C << RESET;
        if (i < 4) cout << ' ';
    }
    cout << '\n';
}

int main(int argc, char** argv) {
    //single declaration of words
    vector<string> words;

    //Load from command-line argument if provided
    if (argc >= 2) {
        words = load_words(argv[1]);
        if (words.empty()) cerr << "Warning: couldn't load words from " << argv[1] << ". Using built-in list.\n";
    }

    //Fallback to default path
    if (words.empty()) {
        string filename = "C:\\Users\\PC\\Downloads\\myWordle\\x64\\Debug\\words.txt";
        words = load_words(filename);
    }

    //Fallback to built-in list
    if (words.empty()) {
        words = { "crane","slate","glory","point","brown","laugh","baker","fling","ghost","apple","mango","tiger","drink","skate","storm" };
    }

    unordered_set<string> valid(words.begin(), words.end()); // valid guesses
    string secret = pick_random(words);

    cout << "Welcome to C++ Wordle! Guess the 5-letter word in 6 attempts.\n";
    cout << "(Guesses must be valid words from the dictionary.)\n\n";

    for (int attempt = 1; attempt <= 6; ++attempt) {
        string guess;
        while (true) {
            cout << "Attempt " << attempt << "/6 - enter guess: ";
            if (!getline(cin, guess)) return 0;
            guess = to_lower_str(trim(guess));
            if (guess.size() != 5) { cout << "Please enter exactly 5 letters.\n"; continue; }
            bool only_letters = true;
            for (char c : guess) if (!isalpha((unsigned char)c)) { only_letters = false; break; }
            if (!only_letters) { cout << "Only letters please.\n"; continue; }
            if (valid.find(guess) == valid.end()) { cout << "Word not in allowed list. Try another.\n"; continue; }
            break;
        }

        auto fb = get_feedback(secret, guess);
        print_feedback(guess, fb);

        bool won = true;
        for (char x : fb) if (x != 'G') { won = false; break; }
        if (won) {
            cout << "You win! The word was \"" << secret << "\".\n";
            return 0;
        }
    }
    cout << "Out of tries — the word was \"" << secret << "\".\n";
    return 0;
}

