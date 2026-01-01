#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static void reverse(char str[], int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

static char* itoa(unsigned int num, char* str, int base) {
	if (base < 2 || base > 16) {
        // Base out of range
        return NULL;
    }
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return str;
    }

    int i = 0;
    int is_negative = 0;
    if (num < 0 && base == 10) {
        is_negative = 1;
        num = -num;
    }

    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num /= base;
    }

    if (is_negative) {
        str[i++] = '-';
    }

    str[i] = '\0'; 
    reverse(str, i);
    return str;
}

int printf(const char *fmt, ...) {
  	va_list args;
	va_start(args,fmt);
	const char *p = fmt;
	int count = 0;
	while(*p)
	{
		if(*p == '%')
		{
			int zero_pad = 0;
			int width = 0;
			p++;
			if(*p == '0'){
				zero_pad = 1;
				p++;
			}
			while(*p >= '0' && *p <= '9'){
				width = width*10 + (*p - '0');
				p++;
			}
			if(*p == 'l'){
				p++;
			}
			switch(*p)
			{
				case 'c':
				{
					int w = va_arg(args,int);
					putch(w);
					count++;
					break;
				}
				case 'd':
				{
					unsigned int i = va_arg(args,unsigned int);
					char t[20];
					int len;
					itoa(i,t,10);
					len = strlen(t);
					int pad_len = width - len;
					if (pad_len > 0) {
						char pad_char = zero_pad ? '0' : ' ';
						for (int i = 0; i < pad_len; i++) {
							putch(pad_char);
							count++;
						}
					}
					char *ptr = t;
					while(*ptr)
					{
						putch(*ptr++);
						count++;
					}	
					break;
				}
				case 's':
				{
					char *j = va_arg(args,char*);
					while(*j){
						putch(*j++);
						count++;
					}	
					break;
				}
				case 'x':
				{
					int u = va_arg(args,unsigned int);
                    char t[20];
                    itoa(u, t, 16);
                    int len = strlen(t);
                    int pad_len = width - len;
                    if (pad_len > 0) {
                        char pad_char = zero_pad ? '0' : ' ';
                        for (int i = 0; i < pad_len; i++) {
                            putch(pad_char);
                            count++;
                        }
                    }
                    char *ptr = t;
                    while (*ptr) {
                        putch(*ptr++);
                        count++;
                    }
                    break;
				}
				case '%':
				{
					putch('%');
					count++;
					break;
				}
				default:
				{
					putch('%');
					putch(*p);
					count += 2;
					break;
				}
			}
		}
		else 
		{
			putch(*p);
			count++;
		}
		p++;
	}
	va_end(args);
	return count;
	//panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
	//return vsnprintf(out,sizeof(out),fmt,ap);
	panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
	va_list args;
    va_start(args, fmt);
    const char *p = fmt;
    char *buf_ptr = out;
    int count = 0;

    while (*p) {
        if (*p == '%') {
            p++;
            switch (*p) {
                case 'd': {
                    int i = va_arg(args, int);
                    char t[20];
                    itoa(i, t, 10);
                    char *ptr = t;
                    while (*ptr) {
                        *buf_ptr++ = *ptr++;
                        count++;
                    }
                    break;
                }
                case 's': {
                    char *j = va_arg(args, char*);
                    while (*j) {
                        *buf_ptr++ = *j++;
                        count++;
                    }
                    break;
                }
                case '%': {
                    *buf_ptr++ = '%';
                    count++;
                    break;
                }
                default: {
                    *buf_ptr++ = '%';
                    *buf_ptr++ = *p;
                    count += 2;
                    break;
                }
            }
        } else {
            *buf_ptr++ = *p;
            count++;
        }
        p++;
    }
    *buf_ptr = '\0'; // Null-terminate the buffer
    va_end(args);
    return count;
      	//panic("Not implemented");
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  	panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
	/*int count = 0;
	const char *p = fmt;
	while(*p && count < n-1)
	{
		if(*p == '%')
		{
			p++;
			switch(*p)
			{
				case 'd':
				{
					int temp = va_arg(ap,int);
       					char num[10];
					itoa(temp,num,10);
					char *nt = num;
					while(*nt)
					{
						*out++ = *nt++;
						count++;
					}	
					break;			
				}
				case 's':
				{
					char *sp = va_arg(ap,char*);
					while(*sp && count < n-1)
					{
						*out++ = *sp++;
						count++;
					}
					break;
				}
				case '%':
				{
					*out = '%';
					out+=1;
					count+=1;
				}
				default:
				{
					*out = '%';
					out+=1;
					*out = *p;
					count+=2;
				}
			}
			p++;
		}
		else 
		{
			*out++ = *p++;
			count++;	
		}
	}
	*out = '\0';
	return count;*/
    panic("Not implemented");
}

#endif
