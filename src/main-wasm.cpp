#include "rules.h"

#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/bind.h>

#include <nlohmann/json.hpp>

Rules rules;

void init(const std::string model_path)
{
    kiwi::KiwiBuilder builder = kiwi::KiwiBuilder(model_path, 0, kiwi::BuildOption::default_, false);
    rules.kiwi = builder.build(kiwi::DefaultTypoSet::withoutTypo);
    rules.loadFromFile(model_path + "/detect-rules");
}

nlohmann::json _analyze(const std::string input)
{
    const std::vector<Token> tokens = rules.analyze(input);

    nlohmann::json result = nlohmann::json::array();

    for (const auto &token : tokens)
    {
        if (token.term.empty())
        {
            result.push_back({
                {"surface", token.surface},
            });
            continue;
        }

        nlohmann::json tokenJson = {
            {"surface", token.surface},
            {"word", token.term},
            {"pos", kiwi::tagToString(token.tag)},
        };

        nlohmann::json suffixes = nlohmann::json::array();
        for (const auto &suffix : token.suffixes)
        {
            suffixes.push_back({
                {"slug", suffix.slug},
                {"start", suffix.start},
                {"end", suffix.end},
            });
        }
        tokenJson["suffixes"] = suffixes;

        result.push_back(tokenJson);
    }

    return result;
}

std::string analyze(const std::string input)
{
    return _analyze(input).dump();
}

std::string analyzeMany(const std::string inputJson)
{
    const nlohmann::json input = nlohmann::json::parse(inputJson);
    nlohmann::json result = nlohmann::json::array();

    for (const auto &inputItem : input)
    {
        result.push_back(_analyze(inputItem));
    }

    return result.dump();
}

EMSCRIPTEN_BINDINGS(grammar)
{
    emscripten::constant("KIWI_VERSION", emscripten::val(KIWI_VERSION_STRING));
    emscripten::function("init", &init);
    emscripten::function("analyze", &analyze);
    emscripten::function("analyzeMany", &analyzeMany);
}
