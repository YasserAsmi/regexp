// Copyright (c) 2014 Yasser Asmi
// Released under the MIT License (http://opensource.org/licenses/MIT)

#include "regexp.h"

#ifndef dbgerr
    #define dbgerr(fmt, ...) \
        do { fprintf(stderr, "Error: %s(%d): " fmt, __FILE__, __LINE__, ## __VA_ARGS__); } while (0)

#endif


using namespace std;

void RegExp::zero()
{
    mRE = NULL;
    mCapCount = 0;
    mNameCount = 0;
    mNameTable = NULL;
    mNameEntrySize = 0;
    mOutVec[0] = 0;
    mOutVec[1] = 0;
    mUtf8 = false;
    mCrlfNewLine = false;
    mSubject = &mInternalSubject;
}

void RegExp::clear()
{
    if (mRE)
    {
        pcre_free(mRE);
    }
    mInternalSubject.clear();
    zero();
}

bool RegExp::compile(const char* pattern)
{
    assert(pattern);

    clear();

    // Compile the regular expression

    int options = 0;
    const char* error;
    int errorofs;

    mRE = pcre_compile(pattern, options, &error, &errorofs, NULL);

    // TODO: pcre_study()?

    if (mRE == NULL)
    {
        // Report error details of the failure.

        dbgerr("Failed to compile PCRE (%s) at %d\n", error, errorofs);
        return false;
    };

    // Determine utf8 and newline

    int optbits = 0;
    (void)pcre_fullinfo(mRE, NULL, PCRE_INFO_OPTIONS, &optbits);

    mUtf8 = optbits & PCRE_UTF8;

    optbits &= PCRE_NEWLINE_CR | PCRE_NEWLINE_LF | PCRE_NEWLINE_CRLF |
        PCRE_NEWLINE_ANY | PCRE_NEWLINE_ANYCRLF;
    if (optbits == 0)
    {
        int d;
        (void)pcre_config(PCRE_CONFIG_NEWLINE, &d);
        optbits = (d == 13) ? PCRE_NEWLINE_CR :
            (d == 10) ? PCRE_NEWLINE_LF :
            (d == (13 << 8 | 10)) ? PCRE_NEWLINE_CRLF :
            (d == -2) ? PCRE_NEWLINE_ANYCRLF :
            (d == -1) ? PCRE_NEWLINE_ANY : 0;
    }

    mCrlfNewLine = optbits == PCRE_NEWLINE_ANY ||
        optbits == PCRE_NEWLINE_CRLF ||
        optbits == PCRE_NEWLINE_ANYCRLF;

    return true;
}


bool RegExp::internalExec(int options, int ofs)
{
    bool match = false;

    // Execute

    mCapCount = 0;
    mNameCount = 0;
    mNameEntrySize = 0;
    mNameTable = NULL;

    int rc = pcre_exec(mRE, NULL, mSubject->c_str(), mSubject->length(), ofs, options, mOutVec, MAX_OUTVEC);

    if (rc < 0)
    {
        if (rc != PCRE_ERROR_NOMATCH)
        {
            dbgerr("PCRE error %d\n", rc);
        }
    }
    else
    {
        match = true;
        if (rc == 0)
        {
            rc = MAX_OUTVEC / 3;
            dbgerr("Too many captured strings, reporting only %d\n", rc - 1);
        }
        mCapCount = rc;

        // Check named substrings

        if (pcre_fullinfo(mRE, NULL, PCRE_INFO_NAMECOUNT, &mNameCount) == 0)
        {
            (void)pcre_fullinfo(mRE, NULL, PCRE_INFO_NAMETABLE, &mNameTable);
            (void)pcre_fullinfo(mRE, NULL, PCRE_INFO_NAMEENTRYSIZE, &mNameEntrySize);
        }
    }

    return match;
}

bool RegExp::exec()
{
    bool match = false;

    if (mRE == NULL)
    {
        dbgerr("Pattern not compiled\n");
        return false;
    }

    if (mOutVec[1] < 0)
    {
        // All matches were found
        return false;
    }

    for (;;)
    {
        int options = 0;
        int startofs = mOutVec[1];

        // If the previous match was for an empty string, we are finished if we are
        // at the end of the string. Otherwise, arrange to run another match at the
        // same point to see if a non-empty match can be found.

        if (mOutVec[0] == mOutVec[1])
        {
            if (mOutVec[0] == (int)mSubject->length())
            {
                break;
            }
            options = PCRE_NOTEMPTY_ATSTART | PCRE_ANCHORED;
        }

        match = internalExec(options, startofs);

        // On subsequent calls, NOMATCH isn't an error. If the value in "options"
        // is zero, it just means we have found all possible matches, so the loop ends.
        // Handle utf8 and crlf options.

        if (!match)
        {
            if (options == 0)
            {
                break;
            }
            mOutVec[1] = startofs + 1;

            if (mCrlfNewLine && (startofs < ((int)mSubject->length() - 1)) &&
                (*mSubject)[startofs] == '\r' &&  (*mSubject)[startofs + 1] == '\n')
            {
                mOutVec[1] += 1;
            }
            else if (mUtf8)
            {
                while (mOutVec[1] < (int)mSubject->length())
                {
                    if (( (*mSubject)[mOutVec[1]] & 0xc0) != 0x80)
                    {
                        break;
                    }
                    mOutVec[1] += 1;
                }
            }

            // Loop again

            continue;

            match = false;
        }

        // No need to loop again

        break;
    }

    return match;
}

std::string RegExp::cap(int cindex)
{
    std::string substr;
    if (cindex >= 0 && cindex < mCapCount)
    {
        int pos = mOutVec[2 * cindex];
        int len = mOutVec[2 * cindex + 1] - mOutVec[2 * cindex];

        substr = mSubject->substr(pos, len);
    }
    return substr;
}


int RegExp::findIndex(const char* cname)
{
    int ret = -1;

    if (mNameCount > 0 && mNameTable != NULL)
    {
        const char* tabptr = mNameTable;
        for (int i = 0; i < mNameCount; i++)
        {
            int n = (tabptr[0] << 8) | tabptr[1];
            int len = mNameEntrySize - 3;

            if (strncmp(cname, tabptr + 2, len) == 0)
            {
                ret = n;
                break;
            }

            tabptr += mNameEntrySize;
        }
    }

    return ret;
}


std::string RegExp::name(int cindex)
{
    std::string s;
    if (mNameCount > 0 && mNameTable != NULL)
    {
        const char* tabptr = mNameTable;
        for (int i = 0; i < mNameCount; i++)
        {
            if (i == cindex)
            {
                s = std::string(tabptr + 2, mNameEntrySize - 3);
                break;
            }
            tabptr += mNameEntrySize;
        }
    }

    return s;
}

void RegExp::capReplace(int cindex, const char* with)
{
    assert(with);
    if (cindex >= 0 && cindex < mCapCount)
    {
        int pos = mOutVec[2 * cindex];
        int len = mOutVec[2 * cindex + 1] - mOutVec[2 * cindex];
        int delta = strlen(with) - len;

        // Replace the substring

        mSubject->replace(pos, len, with);

        // Update any substring positions that are at or after the
        // end of the modified substring
        int lastSubStrCharPos = pos + len - 1;
        for (int n = 0; n < 2 * mCapCount; n++)
        {
            if (mOutVec[n] >= lastSubStrCharPos)
            {
                mOutVec[n] += delta;
            }

        }
    }
}

