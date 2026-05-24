/**
 * @file StringMatch.h
 * @brief 字符串匹配算法模块
 */

#ifndef STRING_MATCH_H
#define STRING_MATCH_H

#include <string>
#include <vector>

// 匹配结果
struct StringMatchResult {
    std::string algorithm;      // 算法名称
    std::string complexity;     // 时间复杂度
    int matches = 0;            // 匹配数
    long long comparisons = 0;  // 比较次数
    double timeMs = 0.0;        // 执行时间（毫秒）
    std::vector<size_t> positions;  // 匹配位置列表
};

class StringMatch {
public:
    // 四种字符串匹配算法
    StringMatchResult bruteForce(const std::string& text,
                                 const std::string& pattern);
    StringMatchResult kmp(const std::string& text, const std::string& pattern);
    StringMatchResult boyerMoore(const std::string& text,
                                 const std::string& pattern);
    StringMatchResult rabinKarp(const std::string& text,
                                const std::string& pattern);

    // 性能对比与可视化
    void compareAlgorithms(const std::string& text, const std::string& pattern);
    void printPerformanceChart(const std::vector<StringMatchResult>& results);

private:
    // KMP 辅助函数
    std::vector<int> computeKMPTable(const std::string& pattern);
};

#endif  // STRING_MATCH_H
