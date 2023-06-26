#include <iostream>


using namespace std;

#include "structures/LedgerRecord.h"

int main() {

    LedgerRecord lR("AAA107", '1', 2, 10);
    lR.print();
}