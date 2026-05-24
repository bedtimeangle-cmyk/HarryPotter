/**
 * @file Trie.h
 * @brief 前缀树（Trie）模块，用于自动补全
 */

#ifndef TRIE_H
#define TRIE_H

#include <string>
#include <vector>
#include <map>
#include <memory>

struct TrieNode {
    std::map<char, std::shared_ptr<TrieNode>> children;
    int frequency = 0;
    bool isEnd = false;
};

class Trie {
public:
    Trie();

    // 插入单词
    void insert(const std::string& word, int frequency = 1);

    // 自动补全（返回前缀匹配的单词列表）
    std::vector<std::string> autocomplete(const std::string& prefix, int limit = 10);

    // 搜索
    bool search(const std::string& word) const;
    bool startsWith(const std::string& prefix) const;

private:
    std::shared_ptr<TrieNode> root_;

    // 辅助方法
    void dfs(const std::shared_ptr<TrieNode>& node, const std::string& currentWord,
             std::vector<std::pair<std::string, int>>& results);
};

#endif  // TRIE_H
