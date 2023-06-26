#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>
#include "structures/LedgerRecord.h"
#include "functions/my_functions.h"
#include "structures/stats.h"

using namespace std;

int main(int argc, char *argv[]) {


    char configfile[128];   // configuration file name

    if(argc == 3) {
        if(strcmp(argv[1], "-l") == 0) {
            strcpy(configfile, argv[2]);
        } else {
            cout<<"Fatal error in mystation main command line argument parsing"<<endl;
        }
    } else {
        cout <<"No config file given in mystation set-up process. BB"<<endl;
        return 1;
    }

    FILE *fp = fopen(configfile, "r");
    if(fp == NULL) {
        cout<<"Could not open config file. Now exiting"<<endl;
        exit(0);
    }

    // Read all config values from config file
    int capacity, parkperiod, mantime, ttime, stattimes, bays, lotperbay;
    fscanf(fp, "%d\n%d\n%d\n%d\n%d\n%d\n%d", &capacity, &parkperiod, &mantime, &ttime, &stattimes, &bays, &lotperbay);
    fclose(fp);

    // Prompt user for automatic bus-processes creation
    cout<<"How many bus processes do you want me to spawn?"<<endl;
    int buses;
    scanf("%d", &buses);

    // Shared Memory

    // Calculate total size
    int semaphore_size, inout_size, ledger_size, stats_size, station_info, total_sharedM_size;
    inout_size = 2*sizeof(int); // inc, out - not used
    station_info = 5*sizeof(int); // bays, lots per bay. inc_type, inc_lot, out_lot
    semaphore_size = 9*sizeof(sem_t); // 9 Semaphores
    ledger_size = bays*lotperbay*sizeof(LedgerRecord); // Ledger - protected by led_mutex
    stats_size = sizeof(stats); // Stats - protected by led_empty semaphore

    total_sharedM_size = inout_size + station_info + semaphore_size + ledger_size + stats_size; // Sum the size needed

    // Allocate the shared memory
    int shmid = shmget(IPC_PRIVATE, total_sharedM_size, 0666);

    // Attach the memory segment
    void *ptr = shmat(shmid, (void *)0, 0);

    // Initialise some values
    void *temp = ptr;

    // Initialise the int values to the right values for error checking from other processes - each time move by 1 int
    *(int *)temp = 0;       // inc
    temp = (int*)temp + 1;
    *(int *)temp = 0;       // out
    temp = (int*)temp + 1;
    *(int *)temp = bays;    // bays
    temp = (int*)temp + 1;
    *(int *)temp = lotperbay;  // lots per bay
    temp = (int*)temp + 1;
    *(int *)temp = 0;   // type
    temp = (int*)temp + 1;
    *(int *)temp = -1;  // inc_lot
    temp = (int*)temp + 1;
    *(int *)temp = -1;  // out_lot

    // Semaphores init
    void *sems = (int*)ptr + 7; // Move by 7 ints
    for(int i = 0; i < 3; i++) { // 3 loops: inits will be repeated
        sem_init((sem_t *)sems, 1, 1); // *_empty semaphores
        sems = (sem_t*)sems + 1;
        sem_init((sem_t *)sems, 1, 0); // *_full semaphores
        sems = (sem_t*)sems + 1;
        if (i < 2) {
            sem_init((sem_t *)sems, 1, 0); // *_mutex semaphores
        } else {                    // Different configuration for led_mutex
            sem_init((sem_t *)sems, 1, 1);
        }
        if (i < 2) {    // Move by 1 semaphore unless you are in the last one: then the ledgerRecords begin
            sems = (sem_t*)sems + 1;
        }

    }

    // Init LedgerRecords status to 'e' : empty
    void *lr = (sem_t *)sems + 9;   // Move by 9 semaphores
    for(int i = 0; i < 3*lotperbay; i++) {
        (*(LedgerRecord *)lr).status = 'e';
        lr = (LedgerRecord *)lr + 1;
    }

    // Initialise all statistic values to 0
    void *stat = (sem_t *)sems + 9; // Move by 9 semaphores
    stat = (LedgerRecord *)stat + bays*lotperbay; // Move by bays*lotperbay LedgeRecords
    (*(stats *)stat).passengers_off = 0;
    (*(stats *)stat).passengers_on = 0;
    (*(stats *)stat).total_park_time = 0;
    (*(stats *)stat).total_type_1_buses = 0;
    (*(stats *)stat).total_type_2_buses = 0;
    (*(stats *)stat).total_type_3_buses = 0;
    (*(stats *)stat).wait_type1 = 0.0;
    (*(stats *)stat).wait_type2 = 0.0;
    (*(stats *)stat).wait_type3 = 0.0;

    // String version of shmid (shared memory id)
    char shmidstr[16];
    sprintf(shmidstr, "%d", shmid);

    int manager = fork(); // Fork for manager
    if(manager == 0) {

        execlp("./station-manager", "./station-manager", "-s", shmidstr, NULL); // Replace with station-manager process
    }
    cout<<"Station-Manager Spawned"<<endl;

    int comptroller = fork(); // Fork for comptroller
    if(comptroller == 0) {
        char ttimestr[16];
        sprintf(ttimestr, "%d", ttime);
        char statttimestr[16];
        sprintf(statttimestr, "%d", stattimes);
        execlp("./comptroller", "./comptroller", "-d", ttimestr, "-t", statttimestr, "-s", shmidstr, NULL); // Replace with comptroller processes
    }
    cout<<"Comptroller Spawned"<<endl;

    for(int b = 0; b < buses; b++) {

        // String versions of the bus arguments
        char typestr[2];
        char incpassengersstr[12];
        char capacitystr[12];
        char parkperiodstr[12];
        char mantimestr[12];

        sprintf(typestr, "%d", randGenerate(3));
        sprintf(incpassengersstr, "%d", randGenerate(capacity));
        sprintf(capacitystr, "%d", capacity);
        sprintf(parkperiodstr, "%d", parkperiod);
        sprintf(mantimestr, "%d", mantime);

        int child = fork();
        if(child == 0) {
            execlp("./bus", "./bus", "-t", typestr, "-n", incpassengersstr, "-c", capacitystr, "-p", parkperiodstr, "-m", mantimestr, "-s", shmidstr, NULL);    // Replace with bus process
        }
        cout<<"Bus spawned!"<<endl;
    }

    sleep(7*60); // Keep children alive for 7 minutes
    kill(manager, SIGKILL);
    kill(comptroller, SIGKILL);
    cout<<"Station-Manager and Comptroller Killed"<<endl;

    // Detach
    if(shmdt(ptr) == -1) {
        cout<<"Detachment in mystation error"<<endl;
    }

    // Free
    if(shmctl(shmid, IPC_RMID, 0) == -1) {
        cout<<"Removal in mystation error"<<endl;
    }
    cout<<"Now exiting mystation"<<endl;
    return 0;
}