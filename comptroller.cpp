#include <iostream>
#include <cstring>
#include <cstdlib>
#include <sys/shm.h>
#include <semaphore.h>
#include <unistd.h>
#include "structures/LedgerRecord.h"
#include "structures/stats.h"

using namespace std;

int main(int argc, char *argv[]) {

    int time = 0, stattimes = 0, shmid;

    char *flag;
    if(argc == 7) {
        for(int i = 1; i < 6; i+=2) {
            flag = argv[i];
            if(strcmp(flag, "-d") == 0) {
                time = atoi(argv[i+1]);
            } else if(strcmp(flag, "-t") == 0) {
                stattimes = atoi(argv[i+1]);
            } else if(strcmp(flag, "-s") == 0) {
                shmid = atoi(argv[i+1]);
            } else {
                cout<<"Fatal error in comptroller main command line argument parsing"<<endl;
            }
        }
    }


    if(stattimes < time) {
        cout<<"Comtroller won't run: Asking for statistics more often than station status"<<endl;
        return 1;
    }

    // Attach the Shared Memory Segment
    void *ptr = shmat(shmid, (void *)0, 0);

    int bays, lots;
    bays = *((int*)ptr + 2);
    lots = *((int*)ptr + 3);

    // Read only the 2 semaphores you need from the shared memory
    void *sems = (int *)ptr + 7;    // Start of semaphores
    sem_t *stats_mutex = (sem_t *)sems + 6; // Get the 7th semaphore
    sem_t *led_mutex = (sem_t *)sems + 8; // Get the 9th semaphore

    void *stat = (sem_t *)sems + 9; // Read the stats structure from the shared memory
    stat = (LedgerRecord *)stat + (bays*lots);


    void *lr = (sem_t *)sems + 9;   // Start of LedgeRecords

    int c = stattimes/time; // How many current stations reads to do before asking for stats

    int j = 0, buses, passengers;
    while (j < 1) { // Infinite loop
        for(int i = 0; i < c; i++) {
            sleep(time);
            // Variable Reset
            lr = (LedgerRecord*) ((sem_t *)sems + 9);
            buses = 0;
            passengers =0;
            void *lr_plates = lr; // Will be used to loop over and print the plate numbers of the parked buses
            cout<<"Current station status:"<<endl;
            sem_wait(led_mutex);    // Lock the ledge to read
                for(int a = 0; a < bays*lots; a++) {    // Calculate how many buses are present and how many passenger got off of them
                    if((*(LedgerRecord *)lr).status == 'p') {
                        buses += 1;
                        passengers += (*(LedgerRecord *)lr).arrival_passengers;
                    }
                    lr = (LedgerRecord *) lr + 1;
                }
                // Print the reads
                cout<<"\tNumber of passengers: "<<passengers<<endl;
                cout<<"\tNumber of buses: "<<buses<<endl;
                cout<<"\tBuses present: "<<endl;
                if(buses == 0) {    // No buses parked
                    cout<<"\t\t None"<<endl;
                } else {    // If there are buses, find them and print them
                    for(int a = 0; a < bays*lots; a++) {
                        if((*(LedgerRecord *)lr_plates).status == 'p') {
                            cout<<"\t\t"<<(*(LedgerRecord *)lr_plates).plate_number<<endl;
                        }
                        lr_plates = (LedgerRecord *)lr_plates + 1; // Move to the next LedgeRecord
                    }
                }

            sem_post(led_mutex);    // Unlock ledge mutex
        }
        sleep(stattimes - c*time); // Sleep the difference and do a statistics reading
        sem_wait(stats_mutex);  // Lock the stats to read
            // Just calculate and print the statistics
            cout<<"Total number of arrival passengers: "<<(*(stats *)stat).passengers_off<<endl;
            cout<<"Total number of departure passengers: "<<(*(stats *)stat).passengers_on<<endl;
            cout<<"Mean time of waiting time for buses of type 1: ";
            if((*(stats *)stat).total_type_1_buses == 0) {
                cout<<"0"<<endl;
            } else {
                cout<<(*(stats *)stat).wait_type1/(*(stats *)stat).total_type_1_buses;
            }
            cout<<"Mean time of waiting time for buses of type 2: ";
            if((*(stats *)stat).total_type_2_buses == 0) {
                cout<<"0"<<endl;
            } else {
                cout<<(*(stats *)stat).wait_type2/(*(stats *)stat).total_type_2_buses;
            }
            cout<<"Mean time of waiting time for buses of type 1: ";
            if((*(stats *)stat).total_type_3_buses == 0) {
                cout<<"0"<<endl;
            } else {
                cout<<(*(stats *)stat).wait_type3/(*(stats *)stat).total_type_3_buses;
            }
            cout<<"Mean time of waiting for buses (all types): ";
            if((*(stats *)stat).total_type_1_buses + (*(stats *)stat).total_type_2_buses + (*(stats *)stat).total_type_3_buses == 0) {
                cout<<"0"<<endl;
            } else {
                cout<<((*(stats *)stat).wait_type1 + (*(stats *)stat).wait_type2 + (*(stats *)stat).wait_type3)/((*(stats *)stat).total_type_1_buses + (*(stats *)stat).total_type_2_buses + (*(stats *)stat).total_type_3_buses);
            }
            cout<<"Mean time of staying for buses (all types): ";
            if((*(stats *)stat).total_type_1_buses + (*(stats *)stat).total_type_2_buses + (*(stats *)stat).total_type_3_buses == 0) {
                cout<<"0"<<endl;
            } else {
                cout<<(*(stats *)stat).total_park_time/((*(stats *)stat).total_type_1_buses + (*(stats *)stat).total_type_2_buses + (*(stats *)stat).total_type_3_buses);
            }
        sem_post(stats_mutex);  // Unlock the stats mutex

        j *= j; // infinite loop
    }

    if(shmdt(ptr) == -1) {
        cout<<"Detachment error in bus"<<endl;
    }

}