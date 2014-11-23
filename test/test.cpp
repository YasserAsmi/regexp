// Copyright (c) 2014 Yasser Asmi
// Released under the MIT License (http://opensource.org/licenses/MIT)

#include "regexp.h"

// Regular expression from http://regex101.com/#pcre
// it will match email addresses and capture 'Username', 'Domain' and 'TLD'
const char* regex_email = "(?'Username'[-\\w\\d\\.]+?)(?:\\s+at\\s+|\\s*@\\s*|\\s*(?:[\\[\\]@]){3}\\s*)"
                          "(?'Domain'[-\\w\\d\\.]*?)\\s*(?:dot|\\.|(?:[\\[\\]dot\\.]){3,5})\\s*(?'TLD'\\w+)";

// Subject string to performance search or replace
const char* str = "Email 1: yasserasmi@live.com;   email 2: yasserasmi@domain2.com  and more email,  email 3: yasmi@domain3.com ...";


void test_single()
{
    // initialize the regexp object with a regular expression (this compiles it as well)
    RegExp reg(regex_email);

    // setup our subject string
    // you can also use refSubject() which just references the string
    reg.copySubject(str);

    // Find a single match by executing the regular expression
    if (reg.exec())
    {
        printf("Match found at %d\n", reg.matchPos());
    }
    else
    {
        printf("Match NOT found\n");
    }
}

void test_multiple()
{
    RegExp reg;

    // We can either pass the reg exp to the constructor or call compile() method
    reg.compile(regex_email);

    // We can compile once and match for multiple subject strings.  We repeat the
    // test 3 times.  reg
    for (int t = 0; t < 3; t++)
    {
        // setup our subject string
        reg.copySubject(str);

        // Find multiple matches by executing the regular expression.  We need to call
        // exec until it returns false (no more matches).

        int n = 0;
        while (reg.exec())
        {
            printf("Match %d at pos %d for test #%d\n", ++n, reg.matchPos(), t);

            // Print captures
            for (int i = 0; i < reg.capCount(); i++)
            {
                printf("Cap %d: '%s'\n", i, reg.cap(i).c_str());
            }

            // Print named captures and their values
            for (int i = 0; i < reg.nameCount(); i++)
            {
                printf("Named %d: %s = '%s'\n", i, reg.name(i).c_str(), reg.cap(reg.name(i)).c_str() );
            }
        }

    }
}


void test_replace()
{

    // initialize the regexp object with a regular expression
    RegExp reg(regex_email);

    // copy our subject string (one we want to find and replace) into the object.  You can also use refSubject() if you don't want to copy.
    const char* str = "Email 1: yasserasmi@live.com;   email 2: yasserasmi@domain2.com  and more email,  email 3: yasmi@domain3.com ...";
    reg.copySubject(str);

    // loop through all occurrences and replace a few captures
    while (reg.exec())
    {
        reg.capReplace("Domain", "superlongdomainforemail");
        reg.capReplace("TLD", "org");
        reg.capReplace("Username", "y-asmi");
    }

    // print old and new string with replacements
    printf("old: `%s`\nnew: `%s`\n\n", str, reg.getSubject().c_str());
}



int main(int argc, char** argv)
{
    test_single();
    test_multiple();
    test_replace();
}