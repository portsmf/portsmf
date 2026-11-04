//! \file strparse.h Header for String_parse class

#include <string>

class String_parse {
public:
    int pos = 0;
    std::string *str = nullptr;
    void init(std::string *s) {
        str = s;
        pos = 0;
    }
    void skip_space();
    char peek();
    void get_nonspace_quoted(std::string &field);

    //! \brief Get the remaining characters, skipping initial spaces and
    //! final return
    void get_remainder(std::string &field);
};

void string_escape(std::string &result, const char *s, const char *quote);
