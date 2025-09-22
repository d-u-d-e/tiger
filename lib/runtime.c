#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int tiger_main();

const char* getchr()
{
  int ch = getchar();
  if(ch == EOF)
  {
    return "";
  }
  char* out = (char*)malloc(2);
  out[0] = (char)(ch);
  out[1] = '\0';
  return out;
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
  return *s;
}

char* chr(char v)
{
  char* out = (char*)malloc(2);
  out[0] = v;
  out[1] = '\0';
  return out;
}

int string_equal(const char* a, const char* b)
{
  return strcmp(a, b) == 0;
}

void* alloc_record(int64_t fields)
{
  return malloc(fields * sizeof(uint64_t));
}

int main()
{
  return tiger_main();
}
