///////////////////////////////////////////////
// Copyright
///////////////////////////////////////////////
//
// String tokenizer (lexer)
// Copyright (c) 2012-2020 Jari Komppa
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokenizer.h"

// undefine to use new[] instead of malloc() for duplicated strings
//#define MALLOC_COPIED_STRINGS

// undefine to skip the trimming of whitespace in tokens
//#define TRIM_STRINGS

Tokenizer::Tokenizer() 
{ 
    mData = NULL; 
    mCount = 0; 
}


Tokenizer::~Tokenizer() 
{ 
	int i;
    for (i = 0; i < mCount; i++)
	{
        free(mData[i]);
	}

    free(mData);
}


int Tokenizer::init(const char *aBuf, const char *aSeparators) 
{
    if (mData != NULL) 
    {
		int i;
        for (i = 0; i < mCount; i++)             
		{
            free(mData[i]); 
		}
        free(mData);
    }

	mCount = 0;

    if (aBuf == NULL || strlen(aBuf) == 0)
	{
		// It's OK to have empty string to tokenize.
		return 1;
	}

    int sepcount = 0;
    while (aSeparators && aSeparators[sepcount] != 0)
    {
		sepcount++;
	}

    // pass 1: count tokens
    
    mCount = 1;
    int i = 0;
	int quote = 0;
    while (aBuf[i] != 0) 
    {   
		if (aBuf[i] == '"')
		{
			quote = !quote;
		}
		if (!quote)
		{
			for (int j = 0; j < sepcount; j++)
			{
				if (aBuf[i] == aSeparators[j])
				{
					mCount++;
				}
			}
		}
        i++;
    }

    int *offsets = (int *)malloc(sizeof(int) * (mCount + 1) * 2);
    int *lengths = (int *)malloc(sizeof(int) * (mCount + 1));
    
	if (offsets == 0 || lengths == 0)
	{
		free(offsets);
		free(lengths);
		mCount = 0;
		return 0;
	}
    
    mCount = 0;
    offsets[0] = 0;
    i = 0;

    // pass 2: count token sizes and offsets
	quote = 0;
    while (aBuf[i] != 0) 
    {   
		if (aBuf[i] == '"')
		{
			quote = !quote;
		}
		if (!quote)
		{
			for (int j = 0; j < sepcount; j++)
			{
				if (aBuf[i] == aSeparators[j])
				{
					mCount++;
					offsets[mCount] = i + 1;
				}
			}
		}
        i++;
    }

	mCount++;
    offsets[mCount] = i + 1;

#ifdef TRIM_STRINGS
	// pass 3: trim
    
    for (i = 0; i < mCount; i++) 
    {
        // left trim
        while (aBuf[offsets[i]] == 32 || aBuf[offsets[i]] == 9) 
		{
            offsets[i]++;
		}

		// Calculate length
        lengths[i] = offsets[i + 1] - (offsets[i] + 1);

		if (lengths[i] < 0)
			lengths[i] = 0;

        // right trim
        while (lengths[i] > 0 && 
              (aBuf[offsets[i] + lengths[i] - 1] == 32 || 
               aBuf[offsets[i] + lengths[i] - 1] == 9))
		{
            lengths[i]--;
		}
    }
#else
    for (i = 0; i < mCount; i++) 
    {
		// Calculate length
        lengths[i] = offsets[i + 1] - (offsets[i] + 1);

		if (lengths[i] < 0)
			lengths[i] = 0;
    }
#endif

    // pass 4: allocate

    mData = (char**)malloc(sizeof(char*) * mCount);

	if (mData == 0)
	{
		free(offsets);
		free(lengths);
		mCount = 0;
		return 0;
	}

	int oom = 0;

    for (i = 0; i < mCount; i++) 
    {
        mData[i] = (char *)malloc(lengths[i] + 1);
		if (mData[i])
		{
	        mData[i][lengths[i]] = 0;
		}
		else
		{
			oom = 1;
		}
    }

	// out of memory
	if (oom)
	{
		for (i = 0; i < mCount; i++)
		{
			free(mData[i]);
		}
		free(mData);
		free(offsets);
		free(lengths);
		mCount = 0;
		return 0;
	}

    // pass 5: copy tokens
    
    for (i = 0; i < mCount; i++)
	{
		int j = offsets[i];
		int c;
        for (c = 0; j < offsets[i] + lengths[i]; j++, c++)
		{
            mData[i][c] = aBuf[j];
		}
	}

	free(offsets);
	free(lengths);
	return 1;
}


int Tokenizer::count() const
{ 
    return mCount; 
}


const char * Tokenizer::get(int aIdx) const
{ 
    if (aIdx >= mCount || aIdx < 0)
	{
		return NULL;
	}

    return (const char*)mData[aIdx]; 
}


char * Tokenizer::dup(int aIdx) 
{ 
    if (aIdx >= mCount || aIdx < 0)
	{
		return NULL;
	}
	char *s;
	int len = (int)strlen(mData[aIdx]);
#ifdef MALLOC_COPIED_STRINGS
	s = (char *)malloc(len+1);
#else
	s = new char[len+1];
#endif
	if (s)
	{
		memcpy(s, mData[aIdx], len);
		s[len] = 0;
	}
    return s; 
}  


int Tokenizer::equals(int aIdx, const char *aString)
{
    if (aIdx > mCount)
	{
		return 0;
	}

    char *s1 = (char*)get(aIdx);
	char *s2 = (char*)aString;

    while (*s1 && *s2 && *s1 == *s2)
    {
        s1++;
        s2++;
    }

    if (!*s1 && !*s2)
	{ 
		return 1;
	}

    return 0;
}


static const char mytoupper(const char a)
{
	
	if (a >= 'a' && a <= 'z')
	{
		return a + ('A' - 'a');
	}
	
	return a;
}


int Tokenizer::equalsNocase(int aIdx, const char *aString)
{
    if (aIdx > mCount) 
	{
		return 0;
	}

    char *s1 = (char*)get(aIdx);
	char *s2 = (char*)aString;

    while (*s1 && *s2 && mytoupper(*s1) == mytoupper(*s2)) 
    {
        s1++;
        s2++;
    }

    if (!*s1 && !*s2)
	{
		return 1;
	}

    return 0;
}
