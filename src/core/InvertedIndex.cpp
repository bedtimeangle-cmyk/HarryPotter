/**
 * @file InvertedIndex.cpp
 * @brief 倒排索引实现
 */

#include "InvertedIndex.h"
#include "TextProcessor.h"
#include <algorithm>
#include <sstream>

InvertedIndex::InvertedIndex() : documentCount_(0), totalTerms_(0) {}

void InvertedIndex::addDocument(int docId, const std::string& bookName,
                                 const std::vector<std::string>& lines) {
    TextProcessor processor;
    DocumentInfo docInfo;
    docInfo.id = docId;
    docInfo.name = bookName;
    docInfo.totalLines = lines.size();
    docInfo.totalWords = 0;

    // 存储完整的行文本用于上下文显示
    documentLines_[docId] = lines;

    int chapter = 1;
    int lineNumber = 0;

    for (const auto& line : lines) {
        lineNumber++;

        // 检测章节（简单启发式：包含"chapter"或"Chapter"）
        std::string lowerLine = processor.toLowerCase(line);
        if (lowerLine.find("chapter") != std::string::npos) {
            try {
                chapter = std::stoi(line.substr(line.find_first_of("0123456789")));
            } catch (...) {}
        }

        // 分词和处理
        std::vector<std::string> tokens = processor.process(line);
        int position = 0;

        for (const auto& token : tokens) {
            if (!token.empty() && token.length() > 0) {
                // 创建 Posting
                Posting posting;
                posting.docId = docId;
                posting.chapter = chapter;
                posting.line = lineNumber;
                posting.position = position;

                // 添加到倒排索引
                index_[token].push_back(posting);
                docInfo.totalWords++;
                totalTerms_++;

                position++;
            }
        }
    }

    documentInfo_[docId] = docInfo;
    documentCount_++;
}

std::vector<Posting> InvertedIndex::search(const std::string& term) const {
    auto it = index_.find(term);
    if (it != index_.end()) {
        return it->second;
    }
    return {};
}

std::vector<Posting> InvertedIndex::searchAND(
    const std::vector<std::string>& terms) const {
    if (terms.empty()) return {};

    // 获取第一个词的posting列表
    auto results = search(terms[0]);
    if (results.empty()) return {};

    // 与其他词的posting列表求交集
    for (size_t i = 1; i < terms.size(); ++i) {
        auto termPostings = search(terms[i]);
        if (termPostings.empty()) return {};

        // 保留同时出现在两个posting列表中的文档
        std::vector<Posting> intersection;
        for (const auto& p1 : results) {
            for (const auto& p2 : termPostings) {
                if (p1.docId == p2.docId && p1.line == p2.line) {
                    intersection.push_back(p1);
                    break;
                }
            }
        }
        results = intersection;
    }

    return results;
}

std::vector<Posting> InvertedIndex::searchOR(
    const std::vector<std::string>& terms) const {
    std::map<std::pair<int, int>, Posting> uniquePostings;  // (docId, line) -> Posting

    for (const auto& term : terms) {
        auto postings = search(term);
        for (const auto& p : postings) {
            auto key = std::make_pair(p.docId, p.line);
            uniquePostings[key] = p;
        }
    }

    std::vector<Posting> results;
    for (const auto& pair : uniquePostings) {
        results.push_back(pair.second);
    }

    return results;
}

std::vector<Posting> InvertedIndex::searchPhrase(
    const std::vector<std::string>& phrase) const {
    if (phrase.empty()) return {};

    // 获取第一个词的posting列表
    auto results = search(phrase[0]);
    if (results.empty()) return {};

    // 对于每个posting，检查后续词是否在相邻位置
    std::vector<Posting> phraseResults;

    for (const auto& p1 : results) {
        bool matched = true;
        int expectedPosition = p1.position + 1;

        for (size_t i = 1; i < phrase.size(); ++i) {
            bool found = false;
            auto termPostings = search(phrase[i]);

            for (const auto& p2 : termPostings) {
                if (p2.docId == p1.docId && p2.line == p1.line &&
                    p2.position == expectedPosition) {
                    found = true;
                    expectedPosition++;
                    break;
                }
            }

            if (!found) {
                matched = false;
                break;
            }
        }

        if (matched) {
            phraseResults.push_back(p1);
        }
    }

    return phraseResults;
}

std::vector<std::pair<std::string, int>> InvertedIndex::getTopTerms(
    size_t n) const {
    std::vector<std::pair<std::string, int>> terms;

    for (const auto& pair : index_) {
        terms.push_back({pair.first, static_cast<int>(pair.second.size())});
    }

    // 按出现次数降序排序
    std::sort(terms.begin(), terms.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

    if (terms.size() > n) {
        terms.resize(n);
    }

    return terms;
}

DocumentInfo InvertedIndex::getDocumentInfo(int docId) const {
    auto it = documentInfo_.find(docId);
    if (it != documentInfo_.end()) {
        return it->second;
    }
    return DocumentInfo();
}

std::vector<std::string> InvertedIndex::getDocumentLines(int docId) const {
    auto it = documentLines_.find(docId);
    if (it != documentLines_.end()) {
        return it->second;
    }
    return {};
}

size_t InvertedIndex::getVocabularySize() const {
    return index_.size();
}

size_t InvertedIndex::getDocumentCount() const {
    return documentCount_;
}

size_t InvertedIndex::getTotalTerms() const {
    return totalTerms_;
}
