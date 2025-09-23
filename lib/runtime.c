#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int tiger_main();

char* consts[256];
int main()
{
  for(int i = 0; i < 256; i++)
  {
    consts[i] = (char*)malloc(1);
    consts[i][0] = (char)i;
    consts[i][1] = '\0';
  }
  return tiger_main();
}

const char* getchr()
{
  int ch = getchar();
  if(ch == EOF)
  {
    return "";
  }
  return consts[ch];
}

void* init_array(uint64_t size, int64_t value)
{
  int64_t* arr = (int64_t*)malloc(size * sizeof(int64_t));
  for(size_t i = 0; i < size; i++)
  {
    arr[i] = value;
  }
  return arr;
}

char* concat(const char* a, const char* b)
{
  size_t lena = strlen(a);
  size_t lenb = strlen(b);
  size_t len = lena + lenb;
  char* out = (char*)malloc(len + 1);
  memcpy(out, a, lena);
  memcpy(out + lena, b, lenb);
  out[len] = '\0';
  return out;
}

void print(const char* s)
{
  printf("%s", s);
}

int ord(const char* s)
{
  if(strlen(s) == 0)
  {
    return -1;
  }
  return s[0];
}

char* chr(uint8_t i)
{
  return consts[i];
}

int string_equal(const char* a, const char* b)
{
  return strcmp(a, b) == 0;
}

void* alloc_record(int64_t fields)
{
  return malloc(fields * sizeof(uint64_t));
}
