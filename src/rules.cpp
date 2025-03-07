#include "rules.h"

#ifndef NO_DIR_LOAD
#include "fkyaml.h"
#endif

#include <filesystem>
#include <iostream>
#include <fstream>

void Rules::insert(const char type, const std::string &slug, const std::string &detect, char post_ruleset)
{
    std::stringstream detect_ss(detect);

    std::string part;
    while (std::getline(detect_ss, part, '|'))
    {
        std::vector<size_t> rule = RuleSet::split_rule(kiwi, part);

        switch (type)
        {
        case 'n':
            noun.insert(slug, rule, post_ruleset);
            break;
        case 'v':
            verb.insert(slug, rule, post_ruleset);
            break;
        default:
            throw std::invalid_argument("Unknown type: " + std::string(1, type));
        }
    }
}

std::vector<std::string> listYamlFiles(const std::string &path)
{
    std::vector<std::string> files;
    for (const auto &entry : std::filesystem::directory_iterator(path))
    {
        if (entry.path().extension() == ".yaml")
        {
            files.push_back(entry.path().string());
        }
    }
    return files;
}

void Rules::load(const std::string &path)
{
    if (std::filesystem::is_directory(path))
        loadFromDir(path);
    else
        loadFromFile(path);
}

void Rules::loadFromDir(const std::string &point_dir)
{
    std::vector<std::string> point_files = listYamlFiles(point_dir);

    for (const auto &file : point_files)
    {
        const std::string slug = std::filesystem::path(file).stem().string();

        std::ifstream ifs(file);
        fkyaml::node root = fkyaml::node::deserialize(ifs);
        const std::string type = root["metadata"]["type"].get_value<std::string>();

        const bool has_detect = root["metadata"].contains("detect");
        if (!has_detect)
            continue;

        std::string detect = root["metadata"]["detect"].get_value<std::string>();

        char post_ruleset = '\0';
        if (root["metadata"].contains("detect_post"))
            post_ruleset = root["metadata"]["detect_post"].get_value<std::string>()[0];

        insert(type[0], slug, detect, post_ruleset);
    }
}

void Rules::loadFromFile(const std::string &path)
{
    std::ifstream ifs(path);

    std::string line;
    while (std::getline(ifs, line))
    {
        size_t colon_idx_1 = line.find(':');
        size_t colon_idx_2 = line.find(':', colon_idx_1 + 1);
        size_t colon_idx_3 = line.find(':', colon_idx_2 + 1);

        if (colon_idx_1 == std::string::npos || colon_idx_2 == std::string::npos || colon_idx_3 == std::string::npos)
            continue;

        std::string slug = line.substr(0, colon_idx_1);
        std::string type = line.substr(colon_idx_1 + 1, colon_idx_2 - colon_idx_1 - 1);
        std::string post_ruleset = line.substr(colon_idx_2 + 1, colon_idx_3 - colon_idx_2 - 1);
        std::string detect = line.substr(colon_idx_3 + 1);

        if (type.empty())
            continue;

        char post_ruleset_char = '\0';
        if (post_ruleset.empty() || post_ruleset == "-")
            post_ruleset_char = '\0';
        else
            post_ruleset_char = post_ruleset[0];

        insert(type[0], slug, detect, post_ruleset_char);
    }
}

void Rules::clear()
{
    noun.clear();
    verb.clear();
}

std::vector<Token> Rules::analyze(const std::string &input) const
{
    const std::u16string input_16 = kiwi::utf8To16(input);
    kiwi::TokenResult result = kiwi.analyze(input_16, kiwi::Match::all | kiwi::Match::splitComplex);

    // Remove plaintext tokens
    std::vector<kiwi::TokenInfo> raw_tokens = result.first;
    raw_tokens.erase(
        std::remove_if(
            raw_tokens.begin(),
            raw_tokens.end(),
            [](const kiwi::TokenInfo &token)
            { return tagIsPlaintext(token.tag); }),
        raw_tokens.end());

    std::vector<Token> tokens; // Result tokens
    size_t text_idx = 0;

    for (size_t token_idx = 0; token_idx < raw_tokens.size(); token_idx++)
    {
        const kiwi::TokenInfo &token = raw_tokens[token_idx];

        // Plaintext between tokens
        if (text_idx < token.position)
        {
            Token t;
            t.surface = kiwi::utf16To8(input_16.substr(text_idx, token.position - text_idx));
            tokens.push_back(t);
            text_idx = token.position;
        }

        const RuleSet *rules = nullptr;
        if (tagIsNoun(token.tag))
            rules = &noun;
        else if (tagIsVerb(token.tag))
            rules = &verb;

        // End index of the current token
        size_t end_idx = token.position + token.length;

        Token t;
        t.term = kiwi::utf16To8(token.str);
        t.tag = token.tag;

        // Check for rule suffixes
        if (rules)
        {
            // Token index to check for suffixes
            size_t match_check_token_idx = token_idx + 1;

            // Result of successful rule match
            RuleSet::MatchResult match_result;

            // Match as many suffixes as possible
            // TODO: Allow ruleset changes after a match
            while (true)
            {
                bool matched = rules->match(match_result, kiwi, raw_tokens, match_check_token_idx, input_16, end_idx);
                if (!matched)
                    break;

                // Token at the start of the match chain
                const kiwi::TokenInfo &match_tokens_first = raw_tokens[match_check_token_idx];
                // Token at the end of the match chain (rules always end with a token, not a plaintext/space segment)
                const kiwi::TokenInfo &match_tokens_last = raw_tokens[match_result.last_token_idx];

                // Update end index to the end of the match chain, also for the next iteration
                end_idx = match_tokens_last.position + match_tokens_last.length;

                // Start next iteration from the next token after the match chain
                match_check_token_idx = match_result.last_token_idx + 1;

                t.suffixes.push_back({
                    .slug = match_result.slug,
                    .start = match_tokens_first.position - token.position,
                    .end = end_idx - token.position,
                });


                switch (match_result.post_ruleset)
                {
                case RuleSet::TYPE_NOUN:
                    rules = &noun;
                    break;
                case RuleSet::TYPE_VERB:
                    rules = &verb;
                    break;
                default:
                    break;
                }
            }

            // Move the token index to the end of the last match chain
            token_idx = match_check_token_idx - 1;
        }

        t.surface = kiwi::utf16To8(input_16.substr(token.position, end_idx - token.position));

        text_idx = end_idx;

        tokens.push_back(t);
    }

    // Plaintext at the end
    if (text_idx < input_16.size())
    {
        Token t;
        t.surface = kiwi::utf16To8(input_16.substr(text_idx));
        tokens.push_back(t);
        text_idx = input_16.size();
    }

    return tokens;
}
