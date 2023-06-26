#pragma once

struct stats {

    int passengers_off;
    int passengers_on;
    int total_type_1_buses;
    int total_type_2_buses;
    int total_type_3_buses;
    int total_park_time;
    double wait_type1;
    double wait_type2;
    double wait_type3;

    stats();
    void print();
};