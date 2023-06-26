#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <sys/shm.h>
#include <semaphore.h>
#include <unistd.h>
#include "structures/LedgerRecord.h"

using namespace std;

int main(int argc, char *argv[]) {


    int shmid;

    if(argc == 3) {
        if(strcmp(argv[1], "-s") == 0) {
            shmid = atoi(argv[2]);
        } else {
            cout<<"Fatal error in stations-manager main command line argument parsing"<<endl;
        }
    } else {
        cout<<"Please give shmid now"<<endl;
        scanf("%d", &shmid);
    }

    // Attach the Shared Memory Segment
    void *ptr = shmat(shmid, (void *)0, 0);
    char *temp = (char*) ptr;

    int inc, out, bays, lots, inc_type, inc_lot, out_lot;

    // Read the shared memory values and each time move by 1 int
    inc = *(int *)temp;
    temp += sizeof(int);
    out = *(int *)temp;
    temp += sizeof(int);
    bays = *(int *)temp;
    temp += sizeof(int);
    lots = *(int *)temp;
    temp += sizeof(int);
    inc_type = *(int *)temp;
    temp += sizeof(int);
    inc_lot = *(int *)temp;
    temp += sizeof(int);
    out_lot = *(int *)temp;

    // Read the semaphores from the shared memory each time moving by 1 semaphore
    sem_t *inc_empty, *inc_full, *inc_mutex, *out_empty, *out_full, *out_mutex, *led_empty, *led_full, *led_mutex;
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

    led_empty = (sem_t *)sems;
    sems = (sem_t*)sems + 1;
    led_full = (sem_t *)sems;
    sems = (sem_t*)sems + 1;
    led_mutex = (sem_t *)sems;

    int itype = -8, ilot = -1, olot; // local versions of variables: inc_type, inc_lot, out_lot
    int i = 0;
    while(i < 2) {  // Infinite loop
        void *lr = (sem_t *)sems + 9;   // Start of LedgerRecords
        int come = sem_trywait(inc_full);   // If there's a bus waiting to enter the station
        if(come == 0) {
            // find position - make a function the following excert
            itype = *((int *)ptr+4);    // Read its type from the shared memory
            cout<<"Station manager - searching position for bus of type "<<itype<<endl;
            if(itype == 1) {    // Search the Ledger for an empty position (no other process edits it at the same time
                for(int i = 0; i < lots; i++){
                    if ((*(LedgerRecord *)lr).status == 'd' || (*(LedgerRecord *)lr).status == 'e') {
                        ilot = i;
                        break;
                    }
                }
            } else if(itype == 2) {
                lr = (LedgerRecord *)lr + lots;
                for(int i = lots; i < 2*lots; i++) {
                    if ((*(LedgerRecord *)lr).status == 'd' || (*(LedgerRecord *)lr).status == 'e') {
                        ilot = i;
                        break;
                    }
                }
            } else { // itype == 3
                lr = (LedgerRecord *)lr + 2*lots;
                for(int i = 2*lots; i < 3*lots; i++) {
                    if ((*(LedgerRecord *)lr).status == 'd' || (*(LedgerRecord *)lr).status == 'e') {
                        ilot = i;
                        break;
                    }
                }
            }   // END OF POSITION SEARCH
            if(ilot >= 0) {
                *((int *)ptr+5) = ilot; // inc-lot pointer to shared memory segment
                sem_wait(led_mutex);    // Lock the ledger and update the record
                (*(LedgerRecord *)lr).status = 'p';
                (*(LedgerRecord *)lr).parking_lot = ilot;
                (*(LedgerRecord *)lr).type = itype;
                (*(LedgerRecord *)lr).arrival_time = time(NULL);
                sem_post(led_mutex);    // Unlock the ledger
                sem_post(inc_mutex);    // Unlock for the bus waiting
            }
        }

        int go = sem_trywait(out_full);
        if(go == 0) {   // If there is a bus waiting to exit the station
            int position = *((int *)ptr+6); // Read its current position (safe - no other process edits it at the same time
            lr = (sem_t *)sems + 9 + position;
            cout<<"Station manager - clearing position: "<<position<<endl;
            sem_wait(led_mutex);    // Lock the ledger and write
            (*(LedgerRecord *)lr).departure_time = time(NULL);
            (*(LedgerRecord *)lr).status = 'd';
            sem_post(led_mutex);    // Unlock the ledger
            sem_post(out_mutex);    // 'Notify' the bus which wants to exit
        }
        i*=i; // infinite loop
    }

    // Detach the Shared Memory Segment
    if(shmdt(ptr) == -1) {
        cout<<"Detachment error in manager"<<endl;
    }
    return 0;
}