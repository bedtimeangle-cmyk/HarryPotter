/**
 * @file SearchEngine.h
 * @brief 搜索引擎模块
 */

#ifndef SEARCH_ENGINE_H
#define SEARCH_ENGINE_H

#include "InvertedIndex.h"
#include <string>
#include <vector>

// 搜索结果
struct SearchResult {
    int docId;           // 文档 ID
    int book;            // 书籍 ID
    int chapter;         // 章节号
    int line;            // 行号
    std::string bookName;  // 书籍名称
};

class SearchEngine {
public:
    SearchEngine(const InvertedIndex& index);

    // 查询执行（自动解析查询类型）
    std::vector<SearchResult> executeQuery(const std::string& query);

    // 基础搜索
    std::vector<SearchResult> search(const std::string& term);
    std::vector<SearchResult> searchAND(const std::vector<std::string>& terms);
    std::vector<SearchResult> searchOR(const std::vector<std::string>& terms);
    std::vector<SearchResult> searchPhrase(const std::string& phrase);

private:
    const InvertedIndex& index_;

    // 查询解析
    std::vector<SearchResult> parseAndQuery(const std::string& query);
    std::vector<SearchResult> parseOrQuery(const std::string& query);
    std::vector<SearchResult> parseNotQuery(const std::string& query);

    // 辅助方法
    std::vector<SearchResult> convertPostingsToResults(
        const std::vector<Posting>& postings);
    std::vector<std::string> splitByKeyword(const std::string& str,
                                             const std::string& keyword);
};

#endif  // SEARCH_ENGINE_H
