RegExp
======
Easier CPP interface to PCRE regex engine with global match and replace.

I was looking around for better regex engine than regcomp for my C/C++ apps.  I found PCRE (Perl Compatible Regular Expression, http://pcre.org/) and am very happy with it.  It is very fast.  PCRE is a C library and has an excellent API (see pcredemo.c in PCRE library source code).  There is also a PCRECPP API but I didn't really like that.  It isn't consistent with the C API and seems overly complex.

The RegExp API is easier to use and is very efficient.  It makes it easy to find multiple matches.  You can also easily access captures and named captures.  You can replace captures and named captures with new substrings.   It works with const char* and std::string.  You can copy a string into the object an operate on it, or you can have the object reference an external string.

You only need two files (regexp.h and regexp.cpp)  that you can include in your own code.

It does depend on PCRE to be installed on your system.  Download PCRE from http://pcre.org/  and build it.  I did the following:

./configure CFLAGS=-Dregcomp=PCREregcomp \
            --prefix=/usr                     \
            --docdir=/usr/share/doc/pcre-8.35 \
            --enable-unicode-properties       \
            --enable-pcre16                   \
            --enable-pcre32                   \
            --enable-pcregrep-libz            \
            --disable-static
make
make install

Take a look at test.cpp for quick sample code.  To build the test app, simply type make in the src directory.

Also, http://regex101.com/#pcre is an excellent resource for learning regular expression.  You can test PCRE expressions the website, and then cut and paste into your regexp code.

Build! Go!
