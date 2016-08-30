#include <stdio.h>      /* printf, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

int main ()
{
  printf ("First number: %d\n", rand()%100);
  //srand ();
  printf ("Random number: %d\n", rand()%100);
  srand (2);
  printf ("Again the first number: %d\n", rand()%100);
  //srand (2);
	printf ("the last number: %d\n", rand()%100);
  return 0;
}
