#ifndef _BASE64_H
#define _BASE64_H

#ifdef	__cplusplus
extern "C" {
#endif

int b64_encode(char *dest, const char *src, int len);
int b64_decode(char *dest, const char *src);

#ifdef	__cplusplus
}
#endif

#endif /* BASE64_H */
