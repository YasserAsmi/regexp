// Copyright (c) 2014 Yasser Asmi
// Released under the MIT License (http://opensource.org/licenses/MIT)

#ifndef _REGEXP_H
#define _REGEXP_H

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <pcre.h>

#include <string>


class RegExp
{
public:
    RegExp()
    {
        zero();
    }
    RegExp(const char* pattern)
    {
        zero();
        compile(pattern);
    }
    ~RegExp()
    {
        clear();
    }

    bool compile(const char* pattern);

    void clear();

    // Subject
    inline void copySubject(const std::string& subjectstr)
    {
        mInternalSubject.assign(subjectstr);
        mSubject = &mInternalSubject;
    }
    inline void copySubject(const char* subjectstr)
    {
        mInternalSubject.assign(subjectstr);
        mSubject = &mInternalSubject;
    }
    inline void refSubject(std::string& subjectstr)
    {
        mInternalSubject.clear();
        mSubject = &subjectstr;
    }
    inline const std::string& getSubject()
    {
        return *mSubject;
    }

    // Execute
    bool exec();

    inline int matchPos()
    {
        return mOutVec[0];
    }

    // Capture
    inline int capCount()
    {
        return mCapCount;
    }
    std::string cap(int cindex);
    inline std::string cap(const char* cname)
    {
        return cap(findIndex(cname));
    }
    inline std::string cap(const std::string& cname)
    {
        return cap(findIndex(cname.c_str()));
    }

    void capReplace(int cindex, const char* with);

    void capReplace(const char* cname, const char* with)
    {
        capReplace(findIndex(cname), with);
    }

    // Named captures
    inline int nameCount()
    {
        return mNameCount;
    }
    std::string name(int cindex);
    int findIndex(const char* cname);

private:
    bool internalExec(int options, int ofs);
    void zero();

private:
    enum
    {
        MAX_OUTVEC = 16 * 3
    };

    std::string mInternalSubject;
    std::string* mSubject;
    pcre* mRE;
    int mCapCount;
    int mOutVec[MAX_OUTVEC];
    int mNameCount;
    int mNameEntrySize;
    char* mNameTable;
    bool mUtf8;
    bool mCrlfNewLine;
};


#endif // _REGEXP_H