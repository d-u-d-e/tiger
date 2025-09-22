#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

extern int tiger_main();

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

int main()
{
  return tiger_main();
}
