#ifndef _STRING_H
#define _STRING_H

PUBLIC void* memcpy(void* pDst, void* pStr, int iSize);
PUBLIC void	 memset(void* p_dst, char ch, int size);
PUBLIC int   strlen(char* p_str);

#define phys_copy memcpy
#define phys_set  memset

#endif
