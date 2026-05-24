/**
 * @file StringMatch.cpp
 * @brief 字符串匹配算法实现
 */

#include "StringMatch.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <cmath>

// ==================== Brute Force ====================
StringMatchResult StringMatch::bruteForce(const std::string& text,
                                           const std::string& pattern) {
    StringMatchResult result;
    result.algorithm = "Brute Force";
    result.complexity = "O(nm)";

    auto start = std::chrono::high_resolution_clock::now();

    int matches = 0;
    long long comparisons = 0;

    for (size_t i = 0; i <= text.length() - pattern.length(); ++i) {
        bool found = true;
        for (size_t j = 0; j < pattern.length(); ++j) {
            comparisons++;
            if (text[i + j] != pattern[j]) {
                found = false;
                break;
            }
        }
        if (found) {
            matches++;
            result.positions.push_back(i);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    result.matches = matches;
    result.comparisons = comparisons;
    result.timeMs = duration.count() / 1000.0;

    return result;
}

// ==================== KMP ====================
std::vector<int> StringMatch::computeKMPTable(const std::string& pattern) {
    std::vector<int> lps(pattern.length(), 0);
    int len = 0;
    int i = 1;

    while (i < pattern.length()) {
        if (pattern[i] == pattern[len]) {
            len++;
            lps[i] = len;
            i++;
        } else {
            if (len != 0) {
                len = lps[len - 1];
            } else {
                lps[i] = 0;
                i++;
            }
        }
    }

    return lps;
}

StringMatchResult StringMatch::kmp(const std::string& text,
                                    const std::string& pattern) {
    StringMatchResult result;
    result.algorithm = "KMP";
    result.complexity = "O(n+m)";

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<int> lps = computeKMPTable(pattern);
    int matches = 0;
    long long comparisons = 0;
    int j = 0;

    for (int i = 0; i < text.length(); ++i) {
        while (j > 0 && text[i] != pattern[j]) {
            comparisons++;
            j = lps[j - 1];
        }

        if (text[i] == pattern[j]) {
            comparisons++;
            j++;
        }

        if (j == pattern.length()) {
            matches++;
            result.positions.push_back(i - j + 1);
            j = lps[j - 1];
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    result.matches = matches;
    result.comparisons = comparisons;
    result.timeMs = duration.count() / 1000.0;

    return result;
}

// ==================== Boyer-Moore ====================
StringMatchResult StringMatch::boyerMoore(const std::string& text,
                                           const std::string& pattern) {
    StringMatchResult result;
    result.algorithm = "Boyer-Moore";
    result.complexity = "O(n/m) to O(nm)";

    auto start = std::chrono::high_resolution_clock::now();

    // 构造坏字符表
    const int ALPHABET_SIZE = 256;
    std::vector<int> badChar(ALPHABET_SIZE, -1);
    for (int i = 0; i < pattern.length(); ++i) {
        badChar[(unsigned char)pattern[i]] = i;
    }

    int matches = 0;
    long long comparisons = 0;
    int i = 0;

    while (i <= text.length() - pattern.length()) {
        int j = pattern.length() - 1;
        bool found = true;

        while (j >= 0 && text[i + j] == pattern[j]) {
            comparisons++;
            j--;
        }

        if (j < 0) {
            matches++;
            result.positions.push_back(i);
            i += (i + pattern.length() < text.length()) ? 1 : 1;
        } else {
            comparisons++;
            int shift = j - badChar[(unsigned char)text[i + j]];
            i += std::max(1, shift);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    result.matches = matches;
    result.comparisons = comparisons;
    result.timeMs = duration.count() / 1000.0;

    return result;
}

// ==================== Rabin-Karp ====================
StringMatchResult StringMatch::rabinKarp(const std::string& text,
                                          const std::string& pattern) {
    StringMatchResult result;
    result.algorithm = "Rabin-Karp";
    result.complexity = "O(n+m) average";

    auto start = std::chrono::high_resolution_clock::now();

    const int PRIME = 101;
    const int BASE = 256;

    int patternHash = 0;
    int textHash = 0;
    int power = 1;
    int matches = 0;
    long long comparisons = 0;

    // 计算 pattern 和第一个窗口的哈希值
    for (int i = 0; i < pattern.length(); ++i) {
        patternHash = (patternHash * BASE + pattern[i]) % PRIME;
        textHash = (textHash * BASE + text[i]) % PRIME;
        if (i < pattern.length() - 1) {
            power = (power * BASE) % PRIME;
        }
    }

    // 滑动窗口
    for (int i = 0; i <= text.length() - pattern.length(); ++i) {
        comparisons++;

        if (textHash == patternHash) {
            // 验证是否真的匹配
            bool found = true;
            for (int j = 0; j < pattern.length(); ++j) {
                if (text[i + j] != pattern[j]) {
                    found = false;
                    break;
                }
            }
            if (found) {
                matches++;
                result.positions.push_back(i);
            }
        }

        // 计算下一个窗口的哈希值
        if (i < text.length() - pattern.length()) {
            textHash = (BASE * (textHash - text[i] * power) + text[i + pattern.length()]) % PRIME;
            if (textHash < 0) textHash += PRIME;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    result.matches = matches;
    result.comparisons = comparisons;
    result.timeMs = duration.count() / 1000.0;

    return result;
}

// ==================== 对比与可视化 ====================
void StringMatch::compareAlgorithms(const std::string& text,
                                     const std::string& pattern) {
    std::cout << "\n";
    std::cout << "╔═══════════════════════════════════════════════════════════╗\n";
    std::cout << "║           字符串匹配算法性能对比报告                          ║\n";
    std::cout << "╠───────────────────┬────────┬────────────┬─────────────────╣\n";
    std::cout << "║ 算法名称          │ 匹配数 │ 比较次数   │ 时间(ms)            ║\n";
    std::cout << "╠───────────────────┼────────┼────────────┼─────────────────╣\n";

    auto rf = bruteForce(text, pattern);
    printf("║ %-17s │ %6d │ %10lld │ %15.3f ║\n",
           "Brute Force", rf.matches, rf.comparisons, rf.timeMs);

    auto rk = kmp(text, pattern);
    printf("║ %-17s │ %6d │ %10lld │ %15.3f ║\n",
           "KMP", rk.matches, rk.comparisons, rk.timeMs);

    auto rb = boyerMoore(text, pattern);
    printf("║ %-17s │ %6d │ %10lld │ %15.3f ║\n",
           "Boyer-Moore", rb.matches, rb.comparisons, rb.timeMs);

    auto rrk = rabinKarp(text, pattern);
    printf("║ %-17s │ %6d │ %10lld │ %15.3f ║\n",
           "Rabin-Karp", rrk.matches, rrk.comparisons, rrk.timeMs);

    std::cout << "╚═══════════════════╧════════╧════════════╧═════════════════╝\n";

    // 验证正确性
    if (rf.matches == rk.matches && rf.matches == rb.matches &&
        rf.matches == rrk.matches) {
        std::cout << "\n✓ 正确性验证: 所有算法结果一致 (" << rf.matches << " 个匹配)\n";
    } else {
        std::cout << "\n✗ 警告: 算法结果不一致！\n";
    }
}

void StringMatch::printPerformanceChart(const std::vector<StringMatchResult>& results) {
    std::cout << "\n性能对比图表 (ASCII):\n";
    std::cout << "比较次数:\n";

    long long maxComparisons = 0;
    for (const auto& r : results) {
        if (r.comparisons > maxComparisons) {
            maxComparisons = r.comparisons;
        }
    }

    for (const auto& r : results) {
        int barLength = (r.comparisons * 50) / maxComparisons;
        std::cout << std::setw(15) << r.algorithm << " : ";
        for (int i = 0; i < barLength; ++i) {
            std::cout << "█";
        }
        std::cout << " (" << r.comparisons << ")\n";
    }

    std::cout << "\n执行时间 (ms):\n";

    double maxTime = 0;
    for (const auto& r : results) {
        if (r.timeMs > maxTime) {
            maxTime = r.timeMs;
        }
    }

    for (const auto& r : results) {
        int barLength = (r.timeMs * 50) / (maxTime > 0 ? maxTime : 1);
        std::cout << std::setw(15) << r.algorithm << " : ";
        for (int i = 0; i < barLength; ++i) {
            std::cout << "▓";
        }
        std::cout << " (" << std::fixed << std::setprecision(3) << r.timeMs
                  << " ms)\n";
    }
}
