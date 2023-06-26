#include <iostream>
#include <cstring>
#include <cstdlib>
#include <semaphore.h>
#include <sys/shm.h>
#include <unistd.h>
#include <cstdio>
#include "structures/LedgerRecord.h"
#include "functions/my_functions.h"
#include "structures/stats.h"

using namespace std;

int main(int argc, char *argv[]) {

    time_t come_time = time(NULL), served_time; // come_time -> time asking to enter bus stations, served_time -> time you were assigned a position
    double diff_time;
    int type, incpassengers = 0, capacity = 0, parkperiod = 0, mantime = 0, shmid;

    char *flag;
    if(argc == 13) {
        for(int i = 1; i < 12; i+=2) {
            flag = argv[i];
            if(strcmp(flag, "-t") == 0) {
                type = atoi(argv[i+1]);
            } else if(strcmp(flag, "-n") == 0) {
                incpassengers = atoi(argv[i+1]);
            } else if(strcmp(flag, "-c") == 0) {
                capacity = atoi(argv[i+1]);
            } else if(strcmp(flag, "-p") == 0) {
                parkperiod = atoi(argv[i+1]);
            } else if(strcmp(flag, "-m") == 0) {
                mantime = atoi(argv[i+1]);
            } else if(strcmp(flag, "-s") == 0) {
                shmid = atoi(argv[i+1]);
            } else {
                cout<<"Fatal error in bus main command line argument parsing"<<endl;
            }
        }
    }


    // Attach the Shared Memory Segment
    void *ptr = shmat(shmid, (void *)0, 0);

    int bays, lots;
    bays = *((int*)ptr + 2);
    lots = *((int*)ptr + 3);

    // Read the semaphores from the shared memory each time moving by 1 semaphore
    sem_t *inc_empty, *inc_full, *inc_mutex, *out_empty, *out_full, *out_mutex, *stats_mutex, *led_full, *led_mutex;
    void *sems = (int *)ptr + 7;

    inc_empty = (sem_t *)sems;
    sems = (sem_t *)sems + 1;
    inc_full = (sem_t *)sems;
    sems = (sem_t *)sems + 1;
    inc_mutex = (sem_t *)sems;
    sems = (sem_t *)sems + 1;

    out_empty = (sem_t *)sems;
    sems = (sem_t*)sems + 1;
    out_full = (sem_t *)sems;
    sems = (sem_t*)sems + 1;
    out_mutex = (sem_t *)sems;
    sems = (sem_t*)sems + 1;

    stats_mutex = (sem_t *)sems;
    sems = (sem_t*)sems + 1;
    led_full = (sem_t *)sems;
    sems = (sem_t*)sems + 1;
    led_mutex = (sem_t *)sems;

    // Read the stats structure from the shared memory
    void *stat = (sem_t *)sems + 9; // Move by 9 semaphores
    stat = (LedgerRecord *)stat + (bays*lots); // Move by bays*lots LedgeRecords


    void *lr = (sem_t *)sems + 9;   // Start of LedgeRecords
    sem_wait(inc_empty); // Lock other buses out
        *((int *)ptr+4) = type; // Declare my type in the shared memory
        sem_post(inc_full); // Assign the task to the manager
        cout<<"Bus with plate number '"<<getppid()<<"' waiting for manager to give me position"<<endl;
        sem_wait(inc_mutex); // Wait for the manager to act
            served_time = time(NULL);
            diff_time = difftime(served_time, come_time); //
            int position = *((int *)ptr+5); // Position to park
            cout<<"Bus with plate number '"<<getppid()<<"': Manager gave me position: "<<position<<endl;
            sleep(mantime); // Maneuver
            lr = (LedgerRecord *)lr + position; // Pointer to the LedgerRecord
        sem_wait(led_mutex); // Only 1 Process To Write on Ledger
            char *pn = (*(LedgerRecord *)lr).plate_number;
            sprintf(pn, "%d",getpid()); // Plate number == process id
            (*(LedgerRecord *)lr).arrival_passengers = incpassengers;
        sem_post(led_mutex); // Unlock Ledger
    sem_post(inc_empty); // Unlock the buses

    cout<<"Bus with plate number '"<<getppid()<<"': Parking for passenger onboarding... "<<endl;
    sleep(parkperiod); // Apobibasi ki epibibasi
    int departure_passengers = randGenerate(capacity);  // Generate a random number for onboarding passengers
    sem_wait(out_empty);    // Lock other buses out
        *((int *)ptr+6) = position; // It is safe: No other process accessing this ledgerRecord for write
        sem_post(out_full);
    cout<<"Bus with plate number '"<<getppid()<<"': Waiting manager signal to exit the station "<<endl;
        sem_wait(out_mutex); // Unblocking means that it's time to leave
        sem_wait(led_mutex);    // Lock the ledger to write
            (*(LedgerRecord *)lr).departure_passengers = departure_passengers;
        sem_post(led_mutex);    // Unlock the ledger
        sem_wait(stats_mutex);  // Lock the statistics to write
            (*(stats *)stat).passengers_off += incpassengers;
            (*(stats *)stat).passengers_on += departure_passengers;
            (*(stats *)stat).total_park_time += parkperiod;
            if(type == 1) {
                (*(stats *)stat).total_type_1_buses += 1;
                (*(stats *)stat).wait_type1 += diff_time;
            } else if(type == 2) {
                (*(stats *)stat).total_type_2_buses += 1;
                (*(stats *)stat).wait_type2 += diff_time;
            } else {
                (*(stats *)stat).total_type_3_buses += 1;
                (*(stats *)stat).wait_type3 += diff_time;
            }
        sem_post(stats_mutex);  // Unlock the statistics
    sem_post(out_empty);    // Other bus processes are up

    cout<<"Bus with plate number '"<<getppid()<<"': I'm gooooooone "<<endl;
    // Detach the Shared Memory Segment
    if(shmdt(ptr) == -1) {
        cout<<"Detachment error in bus"<<endl;
    }

}