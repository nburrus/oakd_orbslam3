//
// Copyright (c) 2022, Nicolas Burrus
// This software may be modified and distributed under the terms
// of the BSD license.  See the LICENSE file for details.
//

#include <regex>
#include <unordered_map>
#include <string>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <exception>

#pragma once

namespace tpl
{

static std::string render_template_string (const std::string& input, const std::unordered_map<std::string, std::string>& varDict)
{
    std::regex vars_regex("\\{\\{ (\\S+) \\}\\}");
    auto words_begin = std::sregex_iterator(input.begin(), input.end(), vars_regex);
    auto words_end = std::sregex_iterator();

    std::string output;
    ssize_t last_end = 0;
    for (std::sregex_iterator i = words_begin; i != words_end; ++i)
    {
        std::smatch match = *i;
        std::string var_name = match[1].str();
        // Add the data before the match.
        output.append(input, last_end, match.position() - last_end);
        auto it = varDict.find(var_name);
        if (it == varDict.end())
        {
            throw std::runtime_error ("Could not find variable " + var_name + " in the dictionary.");
        }
        output += it->second;
        last_end = match.position() + match.length();
    }

    // Add the rest of the string.
    output.append(input, last_end, input.size() - last_end);
    return output;
}

static void render_template_file (const std::string& input_file, const std::string& out_file, const std::unordered_map<std::string, std::string>& varDict)
{
    std::string out_str;

    {
        std::ifstream f(input_file);
        std::string input_str((std::istreambuf_iterator<char>(f)),
                                std::istreambuf_iterator<char>());
        out_str = render_template_string(input_str, varDict);
    }

    {
        std::ofstream f (out_file);
        f << out_str;
    }
}

} // tpl
