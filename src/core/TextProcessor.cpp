/**
 * @file TextProcessor.cpp
 * @brief 文本预处理模块实现
 *
 * 本文件包含学生需要实现的核心函数。
 * 标记为 TODO 的函数需要学生完成实现。
 */

#include "TextProcessor.h"
#include <algorithm>
#include <cctype>
#include <sstream>

TextProcessor::TextProcessor() {
    initStopWords();
}

void TextProcessor::initStopWords() {
    // 常用英文停用词表（框架提供，无需修改）
    stopWords_ = {
        "a", "an", "the", "and", "or", "but", "is", "are", "was", "were",
        "be", "been", "being", "have", "has", "had", "do", "does", "did",
        "will", "would", "could", "should", "may", "might", "must",
        "i", "you", "he", "she", "it", "we", "they", "me", "him", "her",
        "us", "them", "my", "your", "his", "its", "our", "their",
        "this", "that", "these", "those", "what", "which", "who", "whom",
        "in", "on", "at", "to", "for", "of", "with", "by", "from", "as",
        "into", "through", "during", "before", "after", "above", "below",
        "up", "down", "out", "off", "over", "under", "again", "then",
        "once", "here", "there", "when", "where", "why", "how", "all",
        "each", "few", "more", "most", "other", "some", "such", "no",
        "nor", "not", "only", "own", "same", "so", "than", "too", "very",
        "just", "can", "now", "said", "one", "two"
    };
}

// ==================== 框架提供的基础函数（无需修改）====================

std::string TextProcessor::readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::vector<std::string> TextProcessor::readLines(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    return lines;
}

bool TextProcessor::isStopWord(const std::string& word) const {
    return stopWords_.find(word) != stopWords_.end();
}

// ==================== 学生需要实现的函数 ====================

/**
 * 实现 - 转换为小写
 */
std::string TextProcessor::toLowerCase(const std::string& str) const {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return result;
}

/**
 * 实现 - 去除标点符号
 */
std::string TextProcessor::removePunctuation(const std::string& str) const {
    std::string result;
    for (unsigned char c : str) {
        if (std::isalnum(c) || std::isspace(c)) {
            result += c;
        }
    }
    return result;
}

/**
 * 实现 - 分词
 */
std::vector<std::string> TextProcessor::tokenize(const std::string& text) const {
    std::vector<std::string> tokens;
    std::istringstream stream(text);
    std::string word;
    while (stream >> word) {
        if (!word.empty()) {
            tokens.push_back(word);
        }
    }
    return tokens;
}

/**
 * 实现 - 过滤停用词
 */
std::vector<std::string> TextProcessor::removeStopWords(
    const std::vector<std::string>& tokens) {
    std::vector<std::string> result;
    for (const auto& token : tokens) {
        if (!isStopWord(token)) {
            result.push_back(token);
        }
    }
    return result;
}

/**
 * 实现（选做）- 简化版词干提取
 */
std::string TextProcessor::stem(const std::string& word) {
    if (word.length() > 5 && word.substr(word.length() - 3) == "ing") {
        return word.substr(0, word.length() - 3);
    }
    if (word.length() > 4 && word.substr(word.length() - 2) == "ed") {
        return word.substr(0, word.length() - 2);
    }
    if (word.length() > 3 && word.back() == 's' &&
        !(word.length() > 1 && word[word.length() - 2] == 's')) {
        return word.substr(0, word.length() - 1);
    }
    return word;
}

// ==================== 整合函数（无需修改）====================

std::vector<std::string> TextProcessor::process(const std::string& text) {
    // 完整预处理流程：小写 -> 去标点 -> 分词 -> 去停用词
    std::string lower = toLowerCase(text);
    std::string cleaned = removePunctuation(lower);
    std::vector<std::string> tokens = tokenize(cleaned);
    return removeStopWords(tokens);
}
