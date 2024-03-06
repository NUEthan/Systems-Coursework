/* Complete the C version of the driver program for compare. This C code does
 * not need to compile. */

#include <stdio.h>

extern long compare(long, long);

int main(int argc, char *argv[]) {
  if(argc !=3) {  //1 means bad
    printf("Invalid Input.\n");
    return 1;
  }
  long arg1 = strtol(argv[1], NULL, 10); //converts to long ints
  long arg2 = strtol(argv[2], NULL, 10);

  long final = compare(arg1, arg2) //starts the compare

  if(final > 0) {
    printf("greater\n");
  } else if(final == 0) {
    printf("equal\n");
  } else {
    printf("less\n");
  }

  return 0;
}

