/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */
 
//	First checked in on 98/12/03 by John R. McMullen, derived from net.h/mkparse.c.

#include "nsEscape.h"
#include "nsMemory.h"
#include "nsCRT.h"

const int netCharType[256] =
/*	Bit 0		xalpha		-- the alphas
**	Bit 1		xpalpha		-- as xalpha but 
**                             converts spaces to plus and plus to %20
**	Bit 3 ...	path		-- as xalphas but doesn't escape '/'
*/
    /*   0 1 2 3 4 5 6 7 8 9 A B C D E F */
    {    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 0x */
		 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,	/* 1x */
		 0,0,0,0,0,0,0,0,0,0,7,4,0,7,7,4,	/* 2x   !"#$%&'()*+,-./	 */
         7,7,7,7,7,7,7,7,7,7,0,0,0,0,0,0,	/* 3x  0123456789:;<=>?	 */
	     0,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,	/* 4x  @ABCDEFGHIJKLMNO  */
	     /* bits for '@' changed from 7 to 0 so '@' can be escaped   */
	     /* in usernames and passwords in publishing.                */
	     7,7,7,7,7,7,7,7,7,7,7,0,0,0,0,7,	/* 5X  PQRSTUVWXYZ[\]^_	 */
	     0,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,	/* 6x  `abcdefghijklmno	 */
	     7,7,7,7,7,7,7,7,7,7,7,0,0,0,0,0,	/* 7X  pqrstuvwxyz{\}~	DEL */
		 0, };

/* decode % escaped hex codes into character values
 */
#define UNHEX(C) \
    ((C >= '0' && C <= '9') ? C - '0' : \
     ((C >= 'A' && C <= 'F') ? C - 'A' + 10 : \
     ((C >= 'a' && C <= 'f') ? C - 'a' + 10 : 0)))


#define IS_OK(C) (netCharType[((unsigned int) (C))] & (mask))
#define HEX_ESCAPE '%'

//----------------------------------------------------------------------------------------
NS_COM char* nsEscape(const char * str, nsEscapeMask mask)
//----------------------------------------------------------------------------------------
{
    if(!str)
        return NULL;
    return nsEscapeCount(str, (PRInt32)nsCRT::strlen(str), mask, NULL);
}

//----------------------------------------------------------------------------------------
NS_COM char* nsEscapeCount(
    const char * str,
    PRInt32 len,
    nsEscapeMask mask,
    PRInt32* out_len)
//----------------------------------------------------------------------------------------
{
	if (!str)
		return 0;

    int i, extra = 0;
    char* hexChars = "0123456789ABCDEF";

	register const unsigned char* src = (const unsigned char *) str;
    for (i = 0; i < len; i++)
	{
        if (!IS_OK(*src++))
            extra += 2; /* the escape, plus an extra byte for each nibble */
	}

	char* result = (char *)nsMemory::Alloc(len + extra + 1);
    if (!result)
        return 0;

    register unsigned char* dst = (unsigned char *) result;
	src = (const unsigned char *) str;
	if (mask == url_XPAlphas)
	{
	    for (i = 0; i < len; i++)
		{
			unsigned char c = *src++;
			if (IS_OK(c))
				*dst++ = c;
			else if (c == ' ')
				*dst++ = '+'; /* convert spaces to pluses */
			else 
			{
				*dst++ = HEX_ESCAPE;
				*dst++ = hexChars[c >> 4];	/* high nibble */
				*dst++ = hexChars[c & 0x0f];	/* low nibble */
			}
		}
	}
	else
	{
	    for (i = 0; i < len; i++)
		{
			unsigned char c = *src++;
			if (IS_OK(c))
				*dst++ = c;
			else 
			{
				*dst++ = HEX_ESCAPE;
				*dst++ = hexChars[c >> 4];	/* high nibble */
				*dst++ = hexChars[c & 0x0f];	/* low nibble */
			}
		}
	}

    *dst = '\0';     /* tack on eos */
	if(out_len)
		*out_len = dst - (unsigned char *) result;
    return result;
}

//----------------------------------------------------------------------------------------
NS_COM char* nsUnescape(char * str)
//----------------------------------------------------------------------------------------
{
	nsUnescapeCount(str);
	return str;
}

//----------------------------------------------------------------------------------------
NS_COM PRInt32 nsUnescapeCount(char * str)
//----------------------------------------------------------------------------------------
{
    register char *src = str;
    register char *dst = str;

    while (*src)
        if (*src != HEX_ESCAPE)
        	*dst++ = *src++;
        else 	
		{
        	src++; /* walk over escape */
        	if (*src)
            {
            	*dst = UNHEX(*src) << 4;
            	src++;
            }
        	if (*src)
            {
            	*dst = (*dst + UNHEX(*src));
            	src++;
            }
        	dst++;
        }

    *dst = 0;
    return (int)(dst - str);

} /* NET_UnEscapeCnt */


NS_COM char *
nsEscapeHTML(const char * string)
{
  char *rv = (char *) nsMemory::Alloc(nsCRT::strlen(string)*6 + 1); /* The +1 is for the trailing null! */
	char *ptr = rv;

	if(rv)
	  {
		for(; *string != '\0'; string++)
		  {
			if(*string == '<')
			  {
				*ptr++ = '&';
				*ptr++ = 'l';
				*ptr++ = 't';
				*ptr++ = ';';
			  }
			else if(*string == '>')
			  {
				*ptr++ = '&';
				*ptr++ = 'g';
				*ptr++ = 't';
				*ptr++ = ';';
			  }
			else if(*string == '&')
			  {
				*ptr++ = '&';
				*ptr++ = 'a';
				*ptr++ = 'm';
				*ptr++ = 'p';
				*ptr++ = ';';
			  }
			else if (*string == '"')
			  {
				*ptr++ = '&';
				*ptr++ = 'q';
				*ptr++ = 'u';
				*ptr++ = 'o';
				*ptr++ = 't';
				*ptr++ = ';';
			  }			
			else
			  {
				*ptr++ = *string;
			  }
		  }
		*ptr = '\0';
	  }

	return(rv);
}

NS_COM PRUnichar *
nsEscapeHTML2(const PRUnichar *aSourceBuffer, PRInt32 aSourceBufferLen)
{
  // if the caller didn't calculate the length
  if (aSourceBufferLen == -1) {
    aSourceBufferLen = nsCRT::strlen(aSourceBuffer); // ...then I will
  }

  PRUnichar *resultBuffer = (PRUnichar *)nsMemory::Alloc(aSourceBufferLen*6*sizeof(PRUnichar) + sizeof(PRUnichar('\0')));
  PRUnichar *ptr = resultBuffer;

  if (resultBuffer) {
    PRInt32 i;

    for(i = 0; i < aSourceBufferLen; i++) {
      if(aSourceBuffer[i] == '<') {
        *ptr++ = '&';
        *ptr++ = 'l';
        *ptr++ = 't';
        *ptr++ = ';';
      } else if(aSourceBuffer[i] == '>') {
        *ptr++ = '&';
        *ptr++ = 'g';
        *ptr++ = 't';
        *ptr++ = ';';
      } else if(aSourceBuffer[i] == '&') {
        *ptr++ = '&';
        *ptr++ = 'a';
        *ptr++ = 'm';
        *ptr++ = 'p';
        *ptr++ = ';';
      } else if (aSourceBuffer[i] == '"') {
        *ptr++ = '&';
        *ptr++ = 'q';
        *ptr++ = 'u';
        *ptr++ = 'o';
        *ptr++ = 't';
        *ptr++ = ';';
      } else {
        *ptr++ = aSourceBuffer[i];
      }
    }
    *ptr = 0;
  }

  return resultBuffer;
}


