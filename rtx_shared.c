static void strcpy(char* dst, const char* src)
{
  while (*src) {
    *dst++ = *src++;
  }
}