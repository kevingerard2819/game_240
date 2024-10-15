#include <stdio.h>

// Declare the struct
typedef struct {
  float f1;  // 4 bytes
  char c1;   // 1 bytes
  float f2;  // 4 bytes
  char c2;   // 1 byte
} /*__attribute__((packed)) */ my_s;  

int main(void) {

  my_s s;
  printf("Size : %d bytes\n"
         "floats 0x%p 0x%p\n"
         "chars  0x%p 0x%p\n",
         (int)sizeof(s), &s.f1, &s.f2, &s.c1, &s.c2);

  return 0;
}
// CODE EXPLANATION
/*  When packed code line was commented we got output as size : 16 and when it was uncommented 
we got output was size : 10  , first output can be attributed to padding so the size is more , 
while for the second output we see size reduced because packing is implemented because of which
padding was not there so the size is reduced to 10.*/