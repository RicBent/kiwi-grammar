#include "ruleset.h"

#include <cassert>

void RuleSet::insert(const std::string &slug, const std::vector<size_t> &rule, char post_ruleset, size_t idx)
{
    assert(idx <= rule.size());

    if (idx == rule.size())
    {
        this->slug = slug;
        this->post_ruleset = post_ruleset;
        return;
    }

    const size_t id = rule[idx];

    if (sub.find(id) == sub.end())
        sub.emplace(id, RuleSet());

    sub[id].insert(slug, rule, post_ruleset, idx + 1);
}

void RuleSet::print(size_t depth, size_t entry) const
{
    for (size_t i = 0; i < depth; i++)
        std::cout << "  ";

    if (depth != 0)
        std::cout << entry << ": ";

    std::cout << (slug.empty() ? "<>" : slug) << std::endl;
    for (const auto &pair : sub)
        pair.second.print(depth + 1, pair.first);
}

bool RuleSet::match(
    RuleSet::MatchResult &out,
    const kiwi::Kiwi &kiwi,
    const std::vector<kiwi::TokenInfo> &tokens,
    size_t token_idx,
    const std::u16string &input,
    size_t last_token_end_idx) const
{
    // Only traverse sub rules if more tokens are available
    if (token_idx < tokens.size())
    {
        const kiwi::TokenInfo &head = tokens[token_idx];

        ssize_t gap = head.position - last_token_end_idx;

        if (gap <= 0)
        {
            size_t id = kiwi.morphToId(head.morph);

            const auto &sub_rule = sub.find(id);
            if (sub_rule != sub.end())
            {
                bool matched = sub_rule->second.match(out, kiwi, tokens, token_idx + 1, input, head.position + head.length);
                if (matched)
                {
                    return true;
                }
            }
        }
        else if (gap == 1 && input[last_token_end_idx] == ' ')
        {
            const auto &sub_rule = sub.find(ID_SPACE);
            if (sub_rule != sub.end())
            {
                bool matched = sub_rule->second.match(out, kiwi, tokens, token_idx, input, head.position);
                if (matched)
                {
                    return true;
                }
            }
        }
    }

    if (!this->slug.empty())
    {
        out = {
            .slug = this->slug,
            .post_ruleset = this->post_ruleset,
            .last_token_idx = token_idx - 1,
        };
        return true;
    }

    return false;
}

void RuleSet::clear()
{
    sub.clear();
    slug.clear();
}

size_t RuleSet::find_kiwi_id(const kiwi::Kiwi &kiwi, const std::string &in)
{
    const size_t colon_idx = in.find(':');
    const std::string word = in.substr(0, colon_idx);
    kiwi::POSTag tag = kiwi::POSTag::unknown;

    if (colon_idx != std::string::npos)
        tag = kiwi::toPOSTag(kiwi::utf8To16(in.substr(colon_idx + 1)));

    std::vector<const kiwi::Morpheme *> morphemes = kiwi.findMorpheme(kiwi::utf8To16(word), tag);

    if (morphemes.empty())
        return RuleSet::ID_ERROR_NOT_FOUND;

    if (morphemes.size() > 1)
        return RuleSet::ID_ERROR_MULTIPLE;

    return kiwi.morphToId(morphemes[0]);
}

std::vector<size_t> RuleSet::split_rule(const kiwi::Kiwi &kiwi, const std::string &rule)
{
    std::vector<size_t> result;

    size_t idx = 0;
    std::string current = "";

    for (const char c : rule)
    {
        if (c == '+')
        {
            if (!current.empty())
            {
                result.push_back(find_kiwi_id(kiwi, current));
                current = "";
            }
        }
        else if (c == ' ')
        {
            if (!current.empty())
            {
                result.push_back(find_kiwi_id(kiwi, current));
                current = "";
            }
            result.push_back(RuleSet::ID_SPACE);
        }
        else
        {
            current += c;
        }
    }

    if (!current.empty())
    {
        result.push_back(find_kiwi_id(kiwi, current));
    }

    return result;
}
