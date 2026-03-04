#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int tiger_main(uint64_t SL);

char* consts[256];
int main()
{
  for(int i = 0; i < 256; i++)
  {
    consts[i] = (char*)malloc(2);
    consts[i][0] = (char)i;
    consts[i][1] = '\0';
  }
  return tiger_main(0);
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

void* alloc_record(int64_t fields)
{
  return malloc(fields * sizeof(uint64_t));
}

int string_equal(const char* a, const char* b)
{
  return strcmp(a, b) == 0;
}

// Library functions
const char* getchr()
{
  int ch = getchar();
  if(ch == EOF)
  {
    return "";
  }
  return consts[ch];
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
  fflush(stdout);
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

__attribute__((used)) struct
{
  uint64_t sl;
  uint64_t p;
} print_c = {0, (uint64_t)&print};

__attribute__((used)) struct
{
  uint64_t sl;
  uint64_t p;
} chr_c = {0, (uint64_t)&chr};

__attribute__((used)) struct
{
  uint64_t sl;
  uint64_t p;
} ord_c = {0, (uint64_t)&ord};

__attribute__((used)) struct
{
  uint64_t sl;
  uint64_t p;
} concat_c = {0, (uint64_t)&concat};

__attribute__((used)) struct
{
  uint64_t sl;
  uint64_t p;
} getchr_c = {0, (uint64_t)&getchr};