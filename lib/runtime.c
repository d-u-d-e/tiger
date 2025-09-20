#include <stdlib.h>
#include <stdint.h>

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

int main()
{
  return tiger_main();
}
