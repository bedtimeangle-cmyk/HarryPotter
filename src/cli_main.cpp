/**
 * @file cli_main.cpp
 * @brief CLI 版本入口
 */

#include "core/TextProcessor.h"
#include "core/InvertedIndex.h"
#include "core/SearchEngine.h"
#include "core/StringMatch.h"
#include "core/Trie.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>

struct BookInfo {
    int id;
    std::string name;
    int lines;
    int words;
};

class HarryPotterCLI {
public:
    HarryPotterCLI() : docIdCounter_(0) {}

    void run() {
        std::cout << "\n========================================\n";
        std::cout << "  Harry Potter Text Retrieval System\n";
        std::cout << "========================================\n\n";
        std::cout << "Type 'help' for available commands\n\n";

        std::string command;
        while (true) {
            std::cout << "> ";
            if (!std::getline(std::cin, command)) break;

            command.erase(0, command.find_first_not_of(" "));
            command.erase(command.find_last_not_of(" ") + 1);

            if (command.empty()) continue;

            if (command == "help") {
                printHelp();
            } else if (command.substr(0, 4) == "load") {
                handleLoad(command);
            } else if (command.substr(0, 6) == "search") {
                handleSearch(command);
            } else if (command.substr(0, 7) == "context") {
                handleContext(command);
            } else if (command.substr(0, 3) == "top") {
                handleTop(command);
            } else if (command.substr(0, 7) == "compare") {
                handleCompare(command);
            } else if (command.substr(0, 5) == "books") {
                handleBooks(command);
            } else if (command == "quit" || command == "exit") {
                std::cout << "Goodbye!\n";
                break;
            } else if (command.substr(0, 3) == "and") {
                handleAND(command);
            } else if (command.substr(0, 2) == "or") {
                handleOR(command);
            } else {
                std::cout << "Unknown command. Type 'help' for available commands.\n";
            }
        }
    }

private:
    InvertedIndex index_;
    SearchEngine* searchEngine_ = nullptr;
    Trie trie_;
    std::map<int, BookInfo> books_;
    int docIdCounter_;
    std::map<int, std::vector<std::string>> documentLines_;

    void printHelp() {
        std::cout << "\nAvailable Commands:\n"
                  << "  load <file> <name>      - Load a text file\n"
                  << "  search <word>           - Search for a word\n"
                  << "  and <word1> <word2>...  - AND query\n"
                  << "  or <word1> <word2>...   - OR query\n"
                  << "  context <n>             - Show context of result #n\n"
                  << "  top <n>                 - Show top n frequent words\n"
                  << "  compare <word>          - Compare string matching algorithms\n"
                  << "  books                   - List loaded books\n"
                  << "  help                    - Show this help message\n"
                  << "  quit                    - Exit program\n\n";
    }

    void handleLoad(const std::string& command) {
        std::istringstream iss(command);
        std::string cmd, file, name;
        iss >> cmd >> file;

        TextProcessor processor;
        try {
            auto lines = processor.readLines(file);
            if (lines.empty()) {
                std::cout << "Error: File is empty\n";
                return;
            }

            std::getline(iss, name);
            name.erase(0, name.find_first_not_of(" "));
            if (name.empty()) {
                name = file;
            }

            docIdCounter_++;
            index_.addDocument(docIdCounter_, name, lines);
            documentLines_[docIdCounter_] = lines;

            BookInfo info;
            info.id = docIdCounter_;
            info.name = name;
            info.lines = lines.size();
            auto docInfo = index_.getDocumentInfo(docIdCounter_);
            info.words = docInfo.totalWords;
            books_[docIdCounter_] = info;

            searchEngine_ = new SearchEngine(index_);

            auto topTerms = index_.getTopTerms(5000);
            for (const auto& [term, freq] : topTerms) {
                trie_.insert(term, freq);
            }

            std::cout << "Loaded: [" << docIdCounter_ << "] " << name
                      << " - " << lines.size() << " lines\n";
            std::cout << "Index updated: " << index_.getVocabularySize()
                      << " unique words, " << index_.getTotalTerms()
                      << " total terms\n";
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
        }
    }

    void handleSearch(const std::string& command) {
        if (!searchEngine_) {
            std::cout << "Please load a file first\n";
            return;
        }

        std::istringstream iss(command);
        std::string cmd, term;
        iss >> cmd >> term;

        if (term.empty()) {
            std::cout << "Usage: search <word>\n";
            return;
        }

        auto results = searchEngine_->search(term);

        std::cout << "\nFound " << results.size() << " occurrences of '" << term
                  << "':\n\n";

        if (results.empty()) {
            std::cout << "No results found\n";
            return;
        }

        std::cout << std::setw(4) << "#" << std::setw(8) << "Line"
                  << std::setw(10) << "Chapter" << std::setw(30) << "Book\n";
        std::cout << std::string(52, "-") << "\n";

        for (size_t i = 0; i < results.size() && i < 50; ++i) {
            std::cout << std::setw(4) << (i + 1) << std::setw(8)
                      << results[i].line << std::setw(10) << results[i].chapter
                      << std::setw(30) << results[i].bookName << "\n";
        }

        if (results.size() > 50) {
            std::cout << "... (showing first 50 of " << results.size()
                      << " results)\n";
        }

        std::cout << "\nUse 'context <n>' to view context for result #n\n";
    }

    void handleAND(const std::string& command) {
        if (!searchEngine_) {
            std::cout << "Please load a file first\n";
            return;
        }

        std::istringstream iss(command);
        std::string cmd;
        iss >> cmd;

        std::vector<std::string> terms;
        std::string term;
        while (iss >> term) {
            terms.push_back(term);
        }

        if (terms.size() < 2) {
            std::cout << "Usage: and <word1> <word2> [word3]...\n";
            return;
        }

        auto results = searchEngine_->searchAND(terms);

        std::cout << "\nFound " << results.size() << " results for AND query:\n";
        for (const auto& t : terms) std::cout << " '" << t << "'";
        std::cout << "\n\n";

        if (results.empty()) {
            std::cout << "No results found\n";
            return;
        }

        std::cout << std::setw(4) << "#" << std::setw(8) << "Line"
                  << std::setw(10) << "Chapter" << std::setw(30) << "Book\n";
        std::cout << std::string(52, "-") << "\n";

        for (size_t i = 0; i < results.size() && i < 50; ++i) {
            std::cout << std::setw(4) << (i + 1) << std::setw(8)
                      << results[i].line << std::setw(10) << results[i].chapter
                      << std::setw(30) << results[i].bookName << "\n";
        }
    }

    void handleOR(const std::string& command) {
        if (!searchEngine_) {
            std::cout << "Please load a file first\n";
            return;
        }

        std::istringstream iss(command);
        std::string cmd;
        iss >> cmd;

        std::vector<std::string> terms;
        std::string term;
        while (iss >> term) {
            terms.push_back(term);
        }

        if (terms.size() < 2) {
            std::cout << "Usage: or <word1> <word2> [word3]...\n";
            return;
        }

        auto results = searchEngine_->searchOR(terms);

        std::cout << "\nFound " << results.size() << " results for OR query:\n";
        for (const auto& t : terms) std::cout << " '" << t << "'";
        std::cout << "\n\n";

        if (results.empty()) {
            std::cout << "No results found\n";
            return;
        }

        std::cout << std::setw(4) << "#" << std::setw(8) << "Line"
                  << std::setw(10) << "Chapter" << std::setw(30) << "Book\n";
        std::cout << std::string(52, "-") << "\n";

        for (size_t i = 0; i < results.size() && i < 50; ++i) {
            std::cout << std::setw(4) << (i + 1) << std::setw(8)
                      << results[i].line << std::setw(10) << results[i].chapter
                      << std::setw(30) << results[i].bookName << "\n";
        }
    }

    void handleContext(const std::string& command) {
        std::istringstream iss(command);
        std::string cmd;
        int n;
        iss >> cmd >> n;

        if (n <= 0) {
            std::cout << "Usage: context <n> (n >= 1)\n";
            return;
        }

        std::cout << "\nNote: Context feature available in Web version.\n";
    }

    void handleTop(const std::string& command) {
        std::istringstream iss(command);
        std::string cmd;
        int n;
        iss >> cmd >> n;

        if (n <= 0) {
            n = 10;
        }

        auto topTerms = index_.getTopTerms(n);

        std::cout << "\nTop " << topTerms.size() << " most frequent words:\n\n";
        std::cout << std::setw(4) << "#" << std::setw(20) << "Word"
                  << std::setw(15) << "Frequency\n";
        std::cout << std::string(39, "-") << "\n";

        for (size_t i = 0; i < topTerms.size(); ++i) {
            std::cout << std::setw(4) << (i + 1) << std::setw(20)
                      << topTerms[i].first << std::setw(15) << topTerms[i].second
                      << "\n";
        }
    }

    void handleCompare(const std::string& command) {
        std::istringstream iss(command);
        std::string cmd, pattern;
        iss >> cmd >> pattern;

        if (pattern.empty()) {
            std::cout << "Usage: compare <word>\n";
            return;
        }

        std::string text;
        for (const auto& [docId, lines] : documentLines_) {
            for (const auto& line : lines) {
                text += line + " ";
            }
        }

        if (text.length() > 1000000) {
            text = text.substr(0, 1000000);
        }

        StringMatch matcher;
        matcher.compareAlgorithms(text, pattern);
    }

    void handleBooks(const std::string& command) {
        if (books_.empty()) {
            std::cout << "No books loaded\n";
            return;
        }

        std::cout << "\nLoaded Books:\n\n";
        std::cout << std::setw(4) << "ID" << std::setw(40) << "Name"
                  << std::setw(12) << "Lines" << std::setw(12) << "Words\n";
        std::cout << std::string(68, "-") << "\n";

        for (const auto& [id, info] : books_) {
            std::cout << std::setw(4) << info.id << std::setw(40) << info.name
                      << std::setw(12) << info.lines << std::setw(12) << info.words
                      << "\n";
        }
    }
};

int main(int argc, char* argv[]) {
    HarryPotterCLI cli;
    cli.run();
    return 0;
}
