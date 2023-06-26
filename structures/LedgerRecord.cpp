#include "LedgerRecord.h"
#include <cstring>
#include <iostream>
using namespace std;

// TODO include times in the constructor
LedgerRecord::LedgerRecord(char *plate_number, int type, int parking_lot, int arrival_passengers, char status) {

    strcpy(this->plate_number, plate_number);
    this->type = type;
    this->parking_lot = parking_lot;
    this->arrival_passengers = arrival_passengers;
    this->status = status;

}

LedgerRecord::~LedgerRecord() {}

void LedgerRecord::print() {

    cout<<"Printing Ledger Record"<<endl;
    cout<<"Plate number: "<<plate_number<<endl;
    cout<<"Type: "<<type<<endl;
    cout<<"Parking Lot: "<<parking_lot<<endl;
    cout<<"Arrival Passengers: "<<arrival_passengers<<endl;
    cout<<"Status: "<<status<<endl;
}