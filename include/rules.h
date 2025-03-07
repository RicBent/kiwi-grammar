#pragma once

#include "ruleset.h"

struct Suffix
{
    std::string slug; // Slug of the rule
    size_t start;     // Start index of the suffix inside the token surface
    size_t end;       // End index of the suffix inside the token surface
};

struct Token
{
    std::string surface;
    std::string term;
    kiwi::POSTag tag;
    std::vector<Suffix> suffixes;
};

struct Rules
{
    Rules() = default;
    Rules(const Rules &) = delete;

    kiwi::Kiwi kiwi;
    RuleSet noun;
    RuleSet verb;

    void insert(const char type, const std::string &slug, const std::string &detect, char post_ruleset = '\0');

    void clear();
    void load(const std::string &path);
    void loadFromDir(const std::string &dir);
    void loadFromFile(const std::string &path);
    std::vector<Token> analyze(const std::string &input) const;

    static inline bool tagIsPlaintext(kiwi::POSTag tag)
    {
        switch (tag)
        {
        case kiwi::POSTag::sf:
        case kiwi::POSTag::sp:
        case kiwi::POSTag::ss:
        case kiwi::POSTag::sso:
        case kiwi::POSTag::ssc:
        case kiwi::POSTag::se:
        case kiwi::POSTag::so:
        case kiwi::POSTag::sw:
        case kiwi::POSTag::sb:
        case kiwi::POSTag::sl:
        case kiwi::POSTag::sh:
        case kiwi::POSTag::sn:
        case kiwi::POSTag::w_url:
        case kiwi::POSTag::w_email:
        case kiwi::POSTag::w_mention:
        case kiwi::POSTag::w_hashtag:
        case kiwi::POSTag::w_serial:
        case kiwi::POSTag::w_emoji:
            return true;
        default:
            return false;
        }
    }

    static inline bool tagIsNoun(kiwi::POSTag tag)
    {
        switch (tag)
        {
        case kiwi::POSTag::nng:
        case kiwi::POSTag::nnp:
        case kiwi::POSTag::nnb:
        case kiwi::POSTag::nr:
        case kiwi::POSTag::np:
            return true;
        default:
            return false;
        }
    }

    static inline bool tagIsVerb(kiwi::POSTag tag)
    {
        switch (kiwi::clearIrregular(tag))
        {
        case kiwi::POSTag::vv:
        case kiwi::POSTag::va:
        case kiwi::POSTag::vx:
        case kiwi::POSTag::vcp:
        case kiwi::POSTag::vcn:
        case kiwi::POSTag::xsv: // TODO: Check if this is a good idea, 하:XSV should maybe merge?
        case kiwi::POSTag::xsa: // TODO: Check if this is a good idea, 하:XSA should maybe merge?
            return true;
        default:
            return false;
        }
    }
};
