/**
 * @file TextProcessor.h
 * @brief 文本处理模块（分词、去标点、停用词过滤）
 */

#ifndef TEXT_PROCESSOR_H
#define TEXT_PROCESSOR_H

#include <string>
#include <vector>
#include <unordered_set>
#include <fstream>
#include <sstream>

class TextProcessor {
public:
    TextProcessor();

    // 文件读取
    std::string readFile(const std::string& filename);
    std::vector<std::string> readLines(const std::string& filename);

    // 文本处理
    std::string toLowerCase(const std::string& str) const;
    std::string removePunctuation(const std::string& str) const;
    std::vector<std::string> tokenize(const std::string& text) const;
    std::vector<std::string> removeStopWords(const std::vector<std::string>& tokens);
    std::string stem(const std::string& word);

    // 完整处理流程
    std::vector<std::string> process(const std::string& text);

    // 停用词查询
    bool isStopWord(const std::string& word) const;

private:
    void initStopWords();
    std::unordered_set<std::string> stopWords_;
};

#endif  // TEXT_PROCESSOR_H
