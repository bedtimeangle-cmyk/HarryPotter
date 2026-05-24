/**
 * @file SearchEngine.cpp
 * @brief 搜索引擎实现
 */

#include "SearchEngine.h"
#include <algorithm>
#include <cctype>
#include <sstream>

SearchEngine::SearchEngine(const InvertedIndex& index) : index_(index) {}

std::vector<SearchResult> SearchEngine::executeQuery(const std::string& query) {
    // 解析查询类型
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(),
        [](unsigned char c) { return std::tolower(c); });

    if (lowerQuery.find(" and ") != std::string::npos) {
        return parseAndQuery(query);
    } else if (lowerQuery.find(" or ") != std::string::npos) {
        return parseOrQuery(query);
    } else if (lowerQuery.find(" not ") != std::string::npos) {
        return parseNotQuery(query);
    } else if (query[0] == '"' && query[query.length() - 1] == '"') {
        // 短语查询
        std::string phrase = query.substr(1, query.length() - 2);
        return searchPhrase(phrase);
    } else {
        // 单词查询
        return search(query);
    }
}

std::vector<SearchResult> SearchEngine::search(const std::string& term) {
    std::string lowerTerm = term;
    std::transform(lowerTerm.begin(), lowerTerm.end(), lowerTerm.begin(),
        [](unsigned char c) { return std::tolower(c); });

    auto postings = index_.search(lowerTerm);
    return convertPostingsToResults(postings);
}

std::vector<SearchResult> SearchEngine::searchAND(
    const std::vector<std::string>& terms) {
    std::vector<std::string> lowerTerms;
    for (const auto& term : terms) {
        std::string lower = term;
        std::transform(lower.begin(), lower.end(), lower.begin(),
            [](unsigned char c) { return std::tolower(c); });
        lowerTerms.push_back(lower);
    }

    auto postings = index_.searchAND(lowerTerms);
    return convertPostingsToResults(postings);
}

std::vector<SearchResult> SearchEngine::searchOR(
    const std::vector<std::string>& terms) {
    std::vector<std::string> lowerTerms;
    for (const auto& term : terms) {
        std::string lower = term;
        std::transform(lower.begin(), lower.end(), lower.begin(),
            [](unsigned char c) { return std::tolower(c); });
        lowerTerms.push_back(lower);
    }

    auto postings = index_.searchOR(lowerTerms);
    return convertPostingsToResults(postings);
}

std::vector<SearchResult> SearchEngine::searchPhrase(
    const std::string& phrase) {
    // 分解短语为单词
    std::vector<std::string> words;
    std::istringstream iss(phrase);
    std::string word;
    while (iss >> word) {
        // 转小写
        std::transform(word.begin(), word.end(), word.begin(),
            [](unsigned char c) { return std::tolower(c); });
        words.push_back(word);
    }

    auto postings = index_.searchPhrase(words);
    return convertPostingsToResults(postings);
}

std::vector<SearchResult> SearchEngine::parseAndQuery(const std::string& query) {
    std::vector<std::string> terms = splitByKeyword(query, " AND ");
    if (terms.empty()) terms = splitByKeyword(query, " and ");

    return searchAND(terms);
}

std::vector<SearchResult> SearchEngine::parseOrQuery(const std::string& query) {
    std::vector<std::string> terms = splitByKeyword(query, " OR ");
    if (terms.empty()) terms = splitByKeyword(query, " or ");

    return searchOR(terms);
}

std::vector<SearchResult> SearchEngine::parseNotQuery(const std::string& query) {
    size_t pos = query.find(" NOT ");
    if (pos == std::string::npos) {
        pos = query.find(" not ");
    }

    if (pos == std::string::npos) return {};

    std::string term1 = query.substr(0, pos);
    std::string term2 = query.substr(pos + 5);

    auto results1 = search(term1);
    auto results2 = search(term2);

    // 从 results1 中移除在 results2 中出现的文档
    std::vector<SearchResult> finalResults;
    for (const auto& r1 : results1) {
        bool found = false;
        for (const auto& r2 : results2) {
            if (r1.docId == r2.docId && r1.line == r2.line) {
                found = true;
                break;
            }
        }
        if (!found) {
            finalResults.push_back(r1);
        }
    }

    return finalResults;
}

std::vector<SearchResult> SearchEngine::convertPostingsToResults(
    const std::vector<Posting>& postings) {
    std::vector<SearchResult> results;

    for (const auto& p : postings) {
        SearchResult sr;
        sr.docId = p.docId;
        sr.book = p.docId;
        sr.chapter = p.chapter;
        sr.line = p.line;

        auto docInfo = index_.getDocumentInfo(p.docId);
        sr.bookName = docInfo.name;

        results.push_back(sr);
    }

    // 去重
    std::sort(results.begin(), results.end(),
        [](const SearchResult& a, const SearchResult& b) {
            if (a.docId != b.docId) return a.docId < b.docId;
            if (a.chapter != b.chapter) return a.chapter < b.chapter;
            return a.line < b.line;
        });
    results.erase(std::unique(results.begin(), results.end(),
        [](const SearchResult& a, const SearchResult& b) {
            return a.docId == b.docId && a.chapter == b.chapter &&
                   a.line == b.line;
        }),
        results.end());

    return results;
}

std::vector<std::string> SearchEngine::splitByKeyword(
    const std::string& str, const std::string& keyword) {
    std::vector<std::string> result;
    size_t start = 0;
    size_t pos = 0;

    while ((pos = str.find(keyword, start)) != std::string::npos) {
        std::string token = str.substr(start, pos - start);
        // 去除前后空格
        token.erase(0, token.find_first_not_of(" "));
        token.erase(token.find_last_not_of(" ") + 1);
        if (!token.empty()) {
            result.push_back(token);
        }
        start = pos + keyword.length();
    }

    std::string token = str.substr(start);
    token.erase(0, token.find_first_not_of(" "));
    token.erase(token.find_last_not_of(" ") + 1);
    if (!token.empty()) {
        result.push_back(token);
    }

    return result;
}
