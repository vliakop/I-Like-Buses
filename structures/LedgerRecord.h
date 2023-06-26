#pragma once


#include <ctime>

// TODO replace string times(arrivalTime, departure_time) with c-datatype
class LedgerRecord {

public:

    time_t arrival_time;
    time_t departure_time;
    char plate_number[16];
    int type; // valid values: 1, 2, 3
    int parking_lot;
    int arrival_passengers;
    int departure_passengers;
    char status; // valid values: 'p' for parked and 'd' for departured and 'e' empty

    LedgerRecord(char plate_number[], int type, int parking_lot, int arrival_passengers, char status = 'd');
    ~LedgerRecord();
    void print();


};

