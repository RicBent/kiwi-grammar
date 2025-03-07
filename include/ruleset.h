#pragma once

#include <string>
#include <unordered_map>

#include <kiwi/Kiwi.h>

struct RuleSet
{
    RuleSet() = default;
    RuleSet(const RuleSet &) = default;

    // -1 is reserved by Kiwi for invalid morphemes
    // Note that size_t is unsigned!
    static constexpr size_t ID_SPACE = -2;
    static constexpr size_t ID_ERROR_NOT_FOUND = -3;
    static constexpr size_t ID_ERROR_MULTIPLE = -4;

    static constexpr char TYPE_NOUN = 'n';
    static constexpr char TYPE_VERB = 'v';

    std::unordered_map<size_t, RuleSet> sub;
    std::string slug;
    char post_ruleset;

    // Recursively insert a rule into the tree.
    void insert(const std::string &slug, const std::vector<size_t> &rule, char post_ruleset, size_t idx = 0);

    // Print the tree in a human-readable format.
    void print(size_t depth = 0, size_t entry = -1) const;

    struct MatchResult
    {
        std::string slug;
        char post_ruleset;
        size_t last_token_idx;
    };

    // Match a rule against a tokenized input
    bool match(
        MatchResult &out,
        const kiwi::Kiwi &kiwi,
        const std::vector<kiwi::TokenInfo> &tokens,
        size_t token_idx,
        const std::u16string &input,
        size_t last_token_end_idx) const;

    void clear();

    static size_t find_kiwi_id(const kiwi::Kiwi &kiwi, const std::string &in);
    static std::vector<size_t> split_rule(const kiwi::Kiwi &kiwi, const std::string &rule);
};
