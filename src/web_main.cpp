/**
 * @file web_main.cpp
 * @brief Web 版本入口（REST API 服务器）
 */

#include "core/TextProcessor.h"
#include "core/InvertedIndex.h"
#include "core/SearchEngine.h"
#include "core/StringMatch.h"
#include "core/Trie.h"
#include "web/HttpServer.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <memory>
#include <map>
#include <iomanip>

// 全局状态
struct AppState {
    std::unique_ptr<InvertedIndex> index;
    std::unique_ptr<SearchEngine> engine;
    std::unique_ptr<Trie> trie;
    std::map<int, std::vector<std::string>> documentLines;
    std::map<int, std::string> documentNames;
    int docCounter = 0;
} gAppState;

// JSON 辅助函数
std::string jsonEscape(const std::string& str) {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += c;
        }
    }
    return result;
}

// URL解码
std::string urlDecode(const std::string& str) {
    std::string result;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%' && i + 2 < str.length()) {
            int hex;
            std::sscanf(str.c_str() + i + 1, "%2x", &hex);
            result += static_cast<char>(hex);
            i += 2;
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    return result;
}

// API Handler: /api/upload
std::string handleUpload(const std::map<std::string, std::string>& params) {
    auto nameIt = params.find("name");
    auto bodyIt = params.find("body");

    if (nameIt == params.end() || bodyIt == params.end()) {
        return R"({"error":"Missing name or body"})";
    }

    std::string name = urlDecode(nameIt->second);
    std::string body = bodyIt->second;

    // 分行处理
    std::vector<std::string> lines;
    std::istringstream iss(body);
    std::string line;
    while (std::getline(iss, line)) {
        lines.push_back(line);
    }

    if (lines.empty()) {
        return R"({"error":"File is empty"})";
    }

    // 添加文档
    gAppState.docCounter++;
    gAppState.index->addDocument(gAppState.docCounter, name, lines);
    gAppState.documentLines[gAppState.docCounter] = lines;
    gAppState.documentNames[gAppState.docCounter] = name;
    gAppState.engine = std::make_unique<SearchEngine>(*gAppState.index);
    gAppState.trie = std::make_unique<Trie>();

    auto topTerms = gAppState.index->getTopTerms(5000);
    for (const auto& [term, freq] : topTerms) {
        gAppState.trie->insert(term, freq);
    }

    auto docInfo = gAppState.index->getDocumentInfo(gAppState.docCounter);
    return R"({"status":"ok","id":)" + std::to_string(gAppState.docCounter) +
           R"(,"name":")" + jsonEscape(name) + R"(","lines":)" + 
           std::to_string(lines.size()) + R"(,"words":)" + 
           std::to_string(docInfo.totalWords) + R"(})";
}

// API Handler: /api/search
std::string handleSearch(const std::map<std::string, std::string>& params) {
    if (!gAppState.engine) {
        return R"({"error":"No documents loaded","results":[]})";
    }

    auto qIt = params.find("q");
    if (qIt == params.end()) {
        return R"({"error":"Missing query","results":[]})";
    }

    std::string query = urlDecode(qIt->second);
    auto modeIt = params.find("mode");
    std::string mode = (modeIt != params.end()) ? modeIt->second : "single";

    std::vector<SearchResult> results;
    if (mode == "and") {
        std::istringstream iss(query);
        std::vector<std::string> terms;
        std::string term;
        while (iss >> term) terms.push_back(term);
        if (terms.size() > 1) results = gAppState.engine->searchAND(terms);
    } else if (mode == "or") {
        std::istringstream iss(query);
        std::vector<std::string> terms;
        std::string term;
        while (iss >> term) terms.push_back(term);
        if (terms.size() > 1) results = gAppState.engine->searchOR(terms);
    } else if (mode == "phrase") {
        results = gAppState.engine->searchPhrase(query);
    } else {
        results = gAppState.engine->search(query);
    }

    // 构成 JSON 响应
    std::string json = R"({"count":)" + std::to_string(results.size()) +
                       R"(,"query":")" + jsonEscape(query) + R"(","results":[)";

    for (size_t i = 0; i < results.size() && i < 100; ++i) {
        if (i > 0) json += ",";
        json += R"({"book":)" + std::to_string(results[i].book) +
                R"(,"bookName":")" + jsonEscape(results[i].bookName) + 
                R"(","chapter":)" + std::to_string(results[i].chapter) + 
                R"(,"line":)" + std::to_string(results[i].line) + R"(})";
    }

    json += R"(]})";
    return json;
}

// API Handler: /api/context
std::string handleContext(const std::map<std::string, std::string>& params) {
    auto docIt = params.find("doc");
    auto lineIt = params.find("line");

    if (docIt == params.end() || lineIt == params.end()) {
        return R"({"error":"Missing doc or line"})";
    }

    int docId = std::stoi(docIt->second);
    int lineNum = std::stoi(lineIt->second);

    auto linesIt = gAppState.documentLines.find(docId);
    if (linesIt == gAppState.documentLines.end()) {
        return R"({"error":"Document not found"})";
    }

    const auto& lines = linesIt->second;
    std::string prevLine = (lineNum > 1) ? jsonEscape(lines[lineNum - 2]) : "";
    std::string currLine = (lineNum > 0 && lineNum <= lines.size())
                               ? jsonEscape(lines[lineNum - 1]) : "";
    std::string nextLine = (lineNum < lines.size()) ? jsonEscape(lines[lineNum]) : "";

    std::string bookName =
        (gAppState.documentNames.find(docId) != gAppState.documentNames.end())
            ? gAppState.documentNames[docId] : "";

    return R"({"bookName":")" + jsonEscape(bookName) + R"(","line":)" + 
           std::to_string(lineNum) + R"(,"previousParagraph":")" + prevLine + 
           R"(","currentParagraph":")" + currLine + R"(","nextParagraph":")" + 
           nextLine + R"("})";
}

// API Handler: /api/stats
std::string handleStats(const std::map<std::string, std::string>& params) {
    return R"({"documents":)" + std::to_string(gAppState.index->getDocumentCount()) +
           R"(,"vocabulary":)" + std::to_string(gAppState.index->getVocabularySize()) +
           R"(,"totalTerms":)" + std::to_string(gAppState.index->getTotalTerms()) + R"(})";
}

// API Handler: /api/top
std::string handleTop(const std::map<std::string, std::string>& params) {
    int n = 30;
    auto nIt = params.find("n");
    if (nIt != params.end()) {
        n = std::stoi(nIt->second);
    }

    auto topTerms = gAppState.index->getTopTerms(n);
    std::string json = R"({"words":[)";

    for (size_t i = 0; i < topTerms.size(); ++i) {
        if (i > 0) json += ",";
        json += R"({"word":")" + jsonEscape(topTerms[i].first) + 
                R"(","count":)" + std::to_string(topTerms[i].second) + R"(})";
    }

    json += R"(]})";
    return json;
}

// API Handler: /api/autocomplete
std::string handleAutocomplete(const std::map<std::string, std::string>& params) {
    auto prefixIt = params.find("prefix");
    if (prefixIt == params.end()) {
        return R"({"suggestions":[]})";
    }

    std::string prefix = urlDecode(prefixIt->second);
    auto suggestions = gAppState.trie->autocomplete(prefix, 10);

    std::string json = R"({"suggestions":[)";
    for (size_t i = 0; i < suggestions.size(); ++i) {
        if (i > 0) json += ",";
        json += R"(")" + jsonEscape(suggestions[i]) + R"(")";
    }
    json += R"(]})";
    return json;
}

// API Handler: /api/files
std::string handleFiles(const std::map<std::string, std::string>& params) {
    std::string json = R"({"loadedBooks":[)";

    bool first = true;
    for (const auto& [docId, name] : gAppState.documentNames) {
        if (!first) json += ",";
        first = false;

        auto docInfo = gAppState.index->getDocumentInfo(docId);
        json += R"({"id":)" + std::to_string(docId) + R"(,"name":")" +
                jsonEscape(name) + R"(","lines":)" + 
                std::to_string(docInfo.totalLines) + R"(,"words":)" +
                std::to_string(docInfo.totalWords) + R"(})";
    }

    json += R"(]})";
    return json;
}

int main(int argc, char* argv[]) {
    // 初始化
    gAppState.index = std::make_unique<InvertedIndex>();
    gAppState.engine = std::make_unique<SearchEngine>(*gAppState.index);
    gAppState.trie = std::make_unique<Trie>();

    // 创建 HTTP 服务器
    int port = 8080;
    if (argc > 1) {
        port = std::stoi(argv[1]);
    }

    HttpServer server(port);

    // 注册 API 处理程序
    server.registerPostHandler("/api/upload", handleUpload);
    server.registerGetHandler("/api/search", handleSearch);
    server.registerGetHandler("/api/context", handleContext);
    server.registerGetHandler("/api/stats", handleStats);
    server.registerGetHandler("/api/top", handleTop);
    server.registerGetHandler("/api/autocomplete", handleAutocomplete);
    server.registerGetHandler("/api/files", handleFiles);

    std::cout << "Harry Potter Text Retrieval Web Server\n";
    std::cout << "Listening on http://localhost:" << port << "\n";
    std::cout << "Open http://localhost:" << port << "/upload.html to start\n\n";

    server.start();

    return 0;
}
