#include <ctime>
#include <cstdlib>
#include "my_functions.h"

int randGenerate(int upperLimit) {

    srand(time(NULL));
    int r = rand();
    r = r%upperLimit + 1;
    return r;

}