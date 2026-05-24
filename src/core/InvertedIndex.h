/**
 * @file InvertedIndex.h
 * @brief 倒排索引模块
 */

#ifndef INVERTED_INDEX_H
#define INVERTED_INDEX_H

#include <string>
#include <vector>
#include <map>
#include <unordered_map>

// Posting 项（单个出现位置）
struct Posting {
    int docId;      // 文档 ID
    int chapter;    // 章节号
    int line;       // 行号
    int position;   // 行内单词位置
};

// 文档信息
struct DocumentInfo {
    int id = 0;
    std::string name;
    int totalLines = 0;
    int totalWords = 0;
};

class InvertedIndex {
public:
    InvertedIndex();

    // 索引构建
    void addDocument(int docId, const std::string& bookName,
                     const std::vector<std::string>& lines);

    // 搜索
    std::vector<Posting> search(const std::string& term) const;
    std::vector<Posting> searchAND(const std::vector<std::string>& terms) const;
    std::vector<Posting> searchOR(const std::vector<std::string>& terms) const;
    std::vector<Posting> searchPhrase(const std::vector<std::string>& phrase) const;

    // 统计
    std::vector<std::pair<std::string, int>> getTopTerms(size_t n) const;
    DocumentInfo getDocumentInfo(int docId) const;
    std::vector<std::string> getDocumentLines(int docId) const;
    size_t getVocabularySize() const;
    size_t getDocumentCount() const;
    size_t getTotalTerms() const;

private:
    // 核心数据结构
    std::map<std::string, std::vector<Posting>> index_;  // 倒排索引
    std::unordered_map<int, DocumentInfo> documentInfo_;  // 文档信息
    std::unordered_map<int, std::vector<std::string>> documentLines_;  // 文档内容

    size_t documentCount_;
    size_t totalTerms_;
};

#endif  // INVERTED_INDEX_H
