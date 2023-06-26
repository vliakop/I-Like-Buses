#include "stats.h"

#include <iostream>
using namespace std;

// Simple struct for statistics keeping
stats::stats() {

    passengers_off = 0;
    passengers_on = 0;
    total_type_1_buses = 0;
    total_type_2_buses = 0;
    total_type_3_buses = 0;
    total_park_time = 0;
    wait_type1 = 0;
    wait_type2 = 0;
    wait_type3 = 0;
}


void stats::print() {

    cout<<"stat printing"<<endl;
    cout<<passengers_off<<endl;
    cout<<passengers_on<<endl;
    cout<<total_type_1_buses<<endl;
    cout<<total_type_2_buses<<endl;
    cout<<total_type_3_buses<<endl;
    cout<<total_park_time<<endl;
    cout<<wait_type1<<endl;
    cout<<wait_type2<<endl;
    cout<<wait_type3<<endl;
}