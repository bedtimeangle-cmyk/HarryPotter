/**
 * @file StringMatch.cpp
 * @brief 字符串匹配算法模块实现
 */

#include "StringMatch.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <cstring>
#include <vector>
#include <map>
#include <algorithm>

// ==================== Brute Force ====================
StringMatchResult StringMatch::bruteForce(const std::string& text,
                                         const std::string& pattern) {
    StringMatchResult result;
    result.algorithm = "Brute Force";
    result.complexity = "O(nm)";
    result.matches = 0;
    result.comparisons = 0;

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i + pattern.length() <= text.length(); ++i) {
        bool match = true;
        for (size_t j = 0; j < pattern.length(); ++j) {
            result.comparisons++;
            if (text[i + j] != pattern[j]) {
                match = false;
                break;
            }
        }
        if (match) {
            result.matches++;
            result.positions.push_back(i);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    result.timeMs =
        std::chrono::duration<double, std::milli>(end - start).count();

    return result;
}

// ==================== KMP ====================
std::vector<int> StringMatch::computeKMPTable(const std::string& pattern) {
    std::vector<int> lps(pattern.length(), 0);
    int len = 0;
    int i = 1;
    while (i < (int)pattern.length()) {
        if (pattern[i] == pattern[len]) {
            len++;
            lps[i] = len;
            i++;
        } else if (len != 0) {
            len = lps[len - 1];
        } else {
            lps[i] = 0;
            i++;
        }
    }
    return lps;
}

StringMatchResult StringMatch::kmp(const std::string& text,
                                   const std::string& pattern) {
    StringMatchResult result;
    result.algorithm = "KMP";
    result.complexity = "O(n+m)";
    result.matches = 0;
    result.comparisons = 0;

    if (pattern.empty() || text.empty()) {
        return result;
    }

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<int> lps = computeKMPTable(pattern);
    int j = 0;
    for (int i = 0; i < (int)text.length(); ++i) {
        while (j > 0 && text[i] != pattern[j]) {
            result.comparisons++;
            j = lps[j - 1];
        }
        result.comparisons++;
        if (text[i] == pattern[j]) {
            j++;
        }
        if (j == (int)pattern.length()) {
            result.matches++;
            result.positions.push_back(i - j + 1);
            j = lps[j - 1];
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    result.timeMs =
        std::chrono::duration<double, std::milli>(end - start).count();

    return result;
}

// ==================== Boyer-Moore ====================
StringMatchResult StringMatch::boyerMoore(const std::string& text,
                                          const std::string& pattern) {
    StringMatchResult result;
    result.algorithm = "Boyer-Moore";
    result.complexity = "O(n/m)~O(nm)";
    result.matches = 0;
    result.comparisons = 0;

    if (pattern.empty() || text.empty() || pattern.length() > text.length()) {
        return result;
    }

    auto start = std::chrono::high_resolution_clock::now();

    // 构建坏字符表
    std::map<char, int> badCharTable;
    for (size_t i = 0; i < pattern.length(); ++i) {
        badCharTable[pattern[i]] = i;
    }

    size_t i = 0;
    while (i <= text.length() - pattern.length()) {
        int j = pattern.length() - 1;
        while (j >= 0 && text[i + j] == pattern[j]) {
            result.comparisons++;
            j--;
        }
        if (j < 0) {
            result.matches++;
            result.positions.push_back(i);
            i++;
        } else {
            result.comparisons++;
            char badChar = text[i + j];
            int shift = badCharTable.count(badChar) ? j - badCharTable[badChar] : j + 1;
            i += std::max(1, shift);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    result.timeMs =
        std::chrono::duration<double, std::milli>(end - start).count();

    return result;
}

// ==================== Rabin-Karp ====================
StringMatchResult StringMatch::rabinKarp(const std::string& text,
                                         const std::string& pattern) {
    StringMatchResult result;
    result.algorithm = "Rabin-Karp";
    result.complexity = "O(n+m)";
    result.matches = 0;
    result.comparisons = 0;

    if (pattern.empty() || text.empty() || pattern.length() > text.length()) {
        return result;
    }

    auto start = std::chrono::high_resolution_clock::now();

    const long long BASE = 256;
    const long long MOD = 101;

    long long patternHash = 0;
    long long textHash = 0;
    long long basePower = 1;
    for (size_t i = 0; i < pattern.length(); ++i) {
        patternHash = (patternHash * BASE + pattern[i]) % MOD;
        textHash = (textHash * BASE + text[i]) % MOD;
        if (i < pattern.length() - 1) {
            basePower = (basePower * BASE) % MOD;
        }
    }
    for (size_t i = 0; i <= text.length() - pattern.length(); ++i) {
        result.comparisons++;
        if (textHash == patternHash) {
            bool match = true;
            for (size_t j = 0; j < pattern.length(); ++j) {
                if (text[i + j] != pattern[j]) {
                    match = false;
                    break;
                }
            }
            if (match) {
                result.matches++;
                result.positions.push_back(i);
            }
        }
        if (i < text.length() - pattern.length()) {
            textHash = (BASE * (textHash - text[i] * basePower) + text[i + pattern.length()]) % MOD;
            if (textHash < 0) {
                textHash += MOD;
            }
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    result.timeMs =
        std::chrono::duration<double, std::milli>(end - start).count();
    return result;
}

// ==================== 性能表格函数（可选） ====================
void StringMatch::compareAlgorithms(const std::string& text, const std::string& pattern) {
    std::vector<StringMatchResult> results;
    results.push_back(bruteForce(text, pattern));
    results.push_back(kmp(text, pattern));
    results.push_back(boyerMoore(text, pattern));
    results.push_back(rabinKarp(text, pattern));
    printPerformanceChart(results);
}

void StringMatch::printPerformanceChart(const std::vector<StringMatchResult>& results) {
    std::cout << "\n╔════════════════════════════════════════════════════════════╗\n";
    std::cout << "║         字符串匹配算法性能对比报告                            ║\n";
    std::cout << "╠───────────────────┬────────┬────────────┬─────────────────╣\n";
    std::cout << "║ 算法名称          │ 匹配数 │ 比较次数   │ 时间(ms)        ║\n";
    std::cout << "╠───────────────────┼────────┼────────────┼─────────────────╣\n";
    for (const auto& result : results) {
        std::cout << "║ " << std::left << std::setw(17) << result.algorithm
                  << " │ " << std::right << std::setw(6) << result.matches
                  << " │ " << std::right << std::setw(10) << result.comparisons
                  << " │ " << std::right << std::setw(13) << std::fixed
                  << std::setprecision(3) << result.timeMs << " ║\n";
    }
    std::cout << "╚───────────────────┴────────┴────────────┴─────────────────╝\n";
    bool consistent = true;
    int firstMatches = results[0].matches;
    for (const auto& result : results) {
        if (result.matches != firstMatches) {
            consistent = false;
            break;
        }
    }
    if (consistent) {
        std::cout << "\n✓ 正确性验证: 所有算法结果一致 (" << firstMatches << " 个匹配)\n\n";
    } else {
        std::cout << "\n✗ 警告: 算法结果不一致!\n\n";
    }
}
