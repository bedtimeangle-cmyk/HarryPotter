/**
 * @file Similarity.h
 * @brief 相似度计算模块（拼写纠错、模糊搜索）
 */

#ifndef SIMILARITY_H
#define SIMILARITY_H

#include "InvertedIndex.h"
#include <string>
#include <vector>

class Similarity {
public:
    Similarity(const InvertedIndex& index);

    // 编辑距离（Levenshtein Distance）
    int editDistance(const std::string& s1, const std::string& s2) const;

    // 拼写建议（返回最相似的单词）
    std::vector<std::string> getSpellingSuggestions(const std::string& word,
                                                     int threshold = 2);

    // 模糊搜索
    std::vector<Posting> fuzzySearch(const std::string& term, int threshold = 2);

private:
    const InvertedIndex& index_;
};

#endif  // SIMILARITY_H
