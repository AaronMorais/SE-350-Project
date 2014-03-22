#include "rtx_shared.h"

// Copies a maximum of n characters to dst from src.
// Use this instead of strcpy when your buffer is of
// a known size, and not a message envelope.
void strcpyn(char* dst, const char* src, int n) {
	int i = 0;
	while (i < n) {
		dst[i] = src[i];
		if (src[i] == '\0') {
			break;
		}
		i++;
	}
	// Make sure dst is *always* null-terminated, even
	// if we couldn't fit the whole string.
	dst[n-1] = '\0';
}

void strcpy(char* dst, const char* src)
{
	// Length here should be the maximum size safe to copy into
	// (struct msgbuf)->mtext. Thus, we make strcpy safe for the
	// most common use case.
	strcpyn(dst, src, 124);
}

bool strequal(const char* a, const char* b) {
	while (*a && *b) {
		if (*a != *b) {
			return false;
		}
		a++;
		b++;
	}
	return *a == *b;
}
