/**
 * @file Trie.cpp
 * @brief 前缀树实现
 */

#include "Trie.h"
#include <algorithm>

Trie::Trie() : root_(std::make_shared<TrieNode>()) {}

void Trie::insert(const std::string& word, int frequency) {
    auto node = root_;
    for (char c : word) {
        if (node->children.find(c) == node->children.end()) {
            node->children[c] = std::make_shared<TrieNode>();
        }
        node = node->children[c];
    }
    node->isEnd = true;
    node->frequency += frequency;
}

std::vector<std::string> Trie::autocomplete(const std::string& prefix, int limit) {
    auto node = root_;

    // 找到前缀对应的节点
    for (char c : prefix) {
        if (node->children.find(c) == node->children.end()) {
            return {};  // 前缀不存在
        }
        node = node->children[c];
    }

    // DFS 遍历以获取所有单词
    std::vector<std::pair<std::string, int>> allWords;
    dfs(node, prefix, allWords);

    // 按频率排序
    std::sort(allWords.begin(), allWords.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

    // 只返回前 limit 个
    std::vector<std::string> results;
    for (size_t i = 0; i < allWords.size() && i < limit; ++i) {
        results.push_back(allWords[i].first);
    }

    return results;
}

bool Trie::search(const std::string& word) const {
    auto node = root_;
    for (char c : word) {
        if (node->children.find(c) == node->children.end()) {
            return false;
        }
        node = node->children[c];
    }
    return node->isEnd;
}

bool Trie::startsWith(const std::string& prefix) const {
    auto node = root_;
    for (char c : prefix) {
        if (node->children.find(c) == node->children.end()) {
            return false;
        }
        node = node->children[c];
    }
    return true;
}

void Trie::dfs(const std::shared_ptr<TrieNode>& node, const std::string& currentWord,
               std::vector<std::pair<std::string, int>>& results) {
    if (node->isEnd) {
        results.push_back({currentWord, node->frequency});
    }

    for (const auto& [c, child] : node->children) {
        dfs(child, currentWord + c, results);
    }
}
