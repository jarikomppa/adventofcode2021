///////////////////////////////////////////////
// Copyright
///////////////////////////////////////////////
//
// String tokenizer (lexer)
// Copyright (c) 2012 Jari Komppa
//
//
///////////////////////////////////////////////
// License
///////////////////////////////////////////////
// 
//     This software is provided 'as-is', without any express or implied
//     warranty.    In no event will the authors be held liable for any damages
//     arising from the use of this software.
// 
//     Permission is granted to anyone to use this software for any purpose,
//     including commercial applications, and to alter it and redistribute it
//     freely, subject to the following restrictions:
// 
//     1. The origin of this software must not be misrepresented; you must not
//        claim that you wrote the original software. If you use this software
//        in a product, an acknowledgment in the product documentation would be
//        appreciated but is not required.
//     2. Altered source versions must be plainly marked as such, and must not be
//        misrepresented as being the original software.
//     3. This notice may not be removed or altered from any source distribution.
// 
// (eg. same as ZLIB license)
// 
//
///////////////////////////////////////////////
//
// This class splits input string into handy tokens. 

class Tokenizer 
{
public:
	// ctor
    Tokenizer();

	// dtor
    ~Tokenizer();

	// Perform tokenization.
	// Can be called several times to tokenize different data.
	// aBuf does not need to be kept after this call, local copies of the
	// data are taken.
	// returns non-0 if success. 
    int init(const char *aBuf, const char *aSeparators = "=,");

	// Return number of tokens. Tokens may have zero length.
    int count() const;

	// Get reference to a const string. The data will be freed when
	// the tokenizer is freed, or init() is called again.
    const char * get(int aIdx) const;

	// Get a copy of a token
    char * dup(int aIdx);

	// Check if token equals a string
    int equals(int aIdx, const char *aString);

	// Check if token equals a string, with case-insensitive test
    int equalsNocase(int aIdx, const char *aString);

private:
	// Stored token strings
    char **mData;  
	
	// Number of tokens
    int mCount;
};
