#include "rules.h"

#include <iostream>
#include <string>

std::string removeHtmlTags(std::string str)
{
    size_t startpos = 0;
    while ((startpos = str.find('<', startpos)) != std::string::npos)
    {
        size_t endpos = str.find('>', startpos);
        if (endpos == std::string::npos)
        {
            break;
        }
        str.erase(startpos, endpos - startpos + 1);
    }
    return str;
}

int main(int argc, char *argv[])
{
    std::string model_dir = "model";
    std::string rules_path = "detect-rules";
    bool strip_html = false;

    for (int i = 1; i < argc;)
    {
        std::string arg = std::string(argv[i]);
        size_t remaining = argc - i - 1;

        if (arg == "-m" && remaining > 0) {
            model_dir = argv[i + 1];
            i += 2;
        }
        else if (arg == "-r" && remaining > 0) {
            rules_path = argv[i + 1];
            i += 2;
        }
        else if (arg == "-s")
        {
            strip_html = true;
            i += 1;
        }
        else
        {
            std::cerr << "Usage: " << argv[0] << " [-m MODEL_DIR] [-r RULES_PATH] [-s]" << std::endl;
            std::cerr << "  -m MODEL_DIR: Kiwi model directory (default: model)" << std::endl;
            std::cerr << "  -r RULES_PATH: Path to the rules file or directory (default: detect-rules)" << std::endl;
            return 1;
        }
    }

    kiwi::KiwiBuilder builder = kiwi::KiwiBuilder{model_dir, 0, kiwi::BuildOption::default_, false};

    Rules rules;
    rules.kiwi = builder.build(kiwi::DefaultTypoSet::basicTypoSetWithContinualAndLengthening);
    rules.load(rules_path);

    while (true)
    {
        std::string input;
        std::cout << ">> " << std::flush;
        std::getline(std::cin, input);

        if (input == "")
            break;
        if (input == "r")
        {
            rules.clear();
            rules.load(rules_path);
            continue;
        }

        if (strip_html)
        {
            input = removeHtmlTags(input);
        }

        std::vector<Token> tokens = rules.analyze(input);
        for (const auto &token : tokens)
        {
            if (token.term.empty())
            {
                std::cout << "PT : " << token.surface << std::endl;
            }
            else
            {
                std::cout << "TK : " << token.surface << " : " << token.term << ":" << tagToString(token.tag) << std::endl;

                for (const auto &suffix : token.suffixes)
                {
                    std::cout << " + : " << suffix.slug << " (" << suffix.start << "-" << suffix.end << ")" << std::endl;
                }
            }
        }
    }

    return 0;
}
