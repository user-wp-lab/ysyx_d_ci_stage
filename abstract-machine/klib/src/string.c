#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  	int count = 0;
	while(s[count] != '\0')
	{
		count++;
	}
	return count;
	//panic("Not implemented");
}

char *strcpy(char *dst, const char *src) {
	char *ret = dst;
	while(*src)
	{
		*dst = *src;
		dst++;
		src++;
	}
	*dst = '\0';
	return ret;
	//panic("Not implemented");
}

char *strncpy(char *dst, const char *src, size_t n) {
    char *ret = dst;
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        dst[i] = src[i];
    }

    for (; i < n; i++) {
        dst[i] = '\0';
    }
    return ret;
	// panic("Not implemented");
}

char *strcat(char *dst, const char *src) {
  	char *ret = dst;
	int cnt = 0;
	while(dst[cnt])
	{
		cnt++;
	}
	while(*src)
	{
		dst[cnt] = *src;
		cnt++;
		src++;
	}
	dst[cnt] = '\0';
	return ret;
	//panic("Not implemented");
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) {
        return 0;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
	//panic("Not implemented");
}

void *memset(void *s, int c, size_t n) {
    unsigned char *p = (unsigned char *)s;
    while (n--) {
        *p++ = (unsigned char)c;
    }
    return s;
	//panic("Not implemented");
}

void *memmove(void *dst, const void *src, size_t n) {
  	unsigned char *d = (unsigned char *)dst;
	const unsigned char *s = (const unsigned char *)src;
	if(d > s)
	{
		d+=n;
		s+=n;
		while(n--)
		{
			*(--d) = *(--s);
		}
	}
	else 
	{
		while(n--)
		{
			*d++ = *s++;
		}
	}
	return dst;
	//panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
  	unsigned char *p = (unsigned char *)out;
	const unsigned char *s = (const unsigned char *)in;
	while(n--)
	{
		*p++ = *s++;
	}	
	return out;
	//panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n) {
  	const unsigned char *s1_p = (const unsigned char *)s1;
    const unsigned char *s2_p = (const unsigned char *)s2;
    
    while (n--) {
        if (*s1_p != *s2_p) {
            return (*s1_p - *s2_p);
        }
        s1_p++;
        s2_p++;
    }
    return 0;
	//panic("Not implemented");
}

#endif
