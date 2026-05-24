/**
 * @file Similarity.cpp
 * @brief 相似度计算实现
 */

#include "Similarity.h"
#include <algorithm>
#include <cmath>

Similarity::Similarity(const InvertedIndex& index) : index_(index) {}

int Similarity::editDistance(const std::string& s1, const std::string& s2) const {
    int m = s1.length();
    int n = s2.length();

    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1));

    for (int i = 0; i <= m; ++i) dp[i][0] = i;
    for (int j = 0; j <= n; ++j) dp[0][j] = j;

    for (int i = 1; i <= m; ++i) {
        for (int j = 1; j <= n; ++j) {
            if (s1[i - 1] == s2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1];
            } else {
                dp[i][j] = 1 + std::min({dp[i - 1][j],      // 删除
                                         dp[i][j - 1],      // 插入
                                         dp[i - 1][j - 1]});  // 替换
            }
        }
    }

    return dp[m][n];
}

std::vector<std::string> Similarity::getSpellingSuggestions(const std::string& word,
                                                             int threshold) {
    std::vector<std::string> suggestions;

    // 遍历所有词汇（简化版：只比较常用词）
    auto topTerms = index_.getTopTerms(5000);

    for (const auto& [term, freq] : topTerms) {
        int distance = editDistance(word, term);
        if (distance <= threshold && distance > 0) {
            suggestions.push_back(term);
        }
    }

    return suggestions;
}

std::vector<Posting> Similarity::fuzzySearch(const std::string& term,
                                               int threshold) {
    std::map<std::pair<int, int>, Posting> uniquePostings;

    auto topTerms = index_.getTopTerms(5000);

    for (const auto& [candidate, freq] : topTerms) {
        int distance = editDistance(term, candidate);
        if (distance <= threshold) {
            auto postings = index_.search(candidate);
            for (const auto& p : postings) {
                auto key = std::make_pair(p.docId, p.line);
                uniquePostings[key] = p;
            }
        }
    }

    std::vector<Posting> results;
    for (const auto& pair : uniquePostings) {
        results.push_back(pair.second);
    }

    return results;
}
