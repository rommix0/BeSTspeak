#pragma once

#include <string>
#include "utils.hpp"

namespace Bestspeech {
namespace sapi {

struct voice_info
{
    const char* name;
    int index;
    bool is_female;
};

inline constexpr int bst_voice_count = 14;

inline constexpr voice_info bst_voices[bst_voice_count] = {
    {"Fred", 0, false},
    {"Sara", 1, true},
    {"Hary", 2, false},
    {"Wendy", 3, true},
    {"Dexter", 4, false},
    {"Alien", 5, false},
    {"Kit", 6, true},
    {"Bruno", 7, false},
    {"Ghost", 8, false},
    {"Peeper", 9, false},
    {"Dracula", 10, false},
    {"Granny", 11, true},
    {"Martha", 12, true},
    {"Tim", 13, false}
};

class voice_attributes
{
public:
    explicit voice_attributes(int voice_index = 0) noexcept
        : index_(voice_index)
    {
        if (index_ < 0 || index_ >= bst_voice_count) {
            index_ = 0;
        }
    }

    [[nodiscard]] std::wstring get_name() const
    {
        return utils::string_to_wstring(bst_voices[index_].name);
    }

    [[nodiscard]] int get_index() const noexcept
    {
        return index_;
    }

    [[nodiscard]] std::wstring get_age() const
    {
        return L"Adult";
    }

    [[nodiscard]] std::wstring get_gender() const
    {
        return bst_voices[index_].is_female ? L"Female" : L"Male";
    }

    [[nodiscard]] std::wstring get_language() const
    {
        return L"409";
    }

private:
    int index_;
};
}
}
