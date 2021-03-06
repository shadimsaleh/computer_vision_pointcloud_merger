#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <cstdlib>
 
using namespace std;

#define NUM_KINECTS 2

struct CurrFrame {
	bool processed;
	int frame_data;
};

CurrFrame curr_frames[NUM_KINECTS];
pthread_mutex_t access_frame_mutexes[NUM_KINECTS];

void lock(int kinectid)
{
	pthread_mutex_lock(&(access_frame_mutexes[kinectid]));
	cout << ">>> mutex " << kinectid << " locked" << endl;
}

void unlock(int kinectid)
{
	pthread_mutex_unlock(&(access_frame_mutexes[kinectid]));
	cout << "<<< mutex " << kinectid << " unlocked" << endl;
}

void *kinect_reader(void *threadid)
{
   long tid;
   int count = 0;
   tid = (long)threadid;
   cout << "Thread for kinect reader n. " << tid << " started" << endl;
   while(true) {
   	   count++;
   	   int waiting_time = 1 + rand() % 4; // max 4 seconds
	   cout << "Thread n. " << tid << " will wait for " << waiting_time << " seconds (before getting the frame)" << endl;
	   sleep(waiting_time);
	   
	   lock(tid);
	   // got the frame
	   curr_frames[tid].processed = false;
	   curr_frames[tid].frame_data = count; // FIXME: replace count with the actual data
	   unlock(tid);
   }
}

int process_frame(int kinectid, int frame_data) {
	cout << "processing frame " << kinectid << endl;
}

int get_frame(int kinectid) {
	cout << "get frame " << kinectid << endl;
	curr_frames[kinectid].processed = true; // will skip (and not process) this frame next time
	int ret = curr_frames[kinectid].frame_data;
	curr_frames[kinectid].frame_data = 0; // empty it
	return ret;
}

int poll_kinect_statuses()
{
	cout << "checking kinect statuses.." << endl;
	int i;
	for (i = 0; i < NUM_KINECTS; i++) {
		lock(i);
		if(curr_frames[i].processed == false) {
			cout << "found frame not processed for kinect n." << i << endl;
			int frame_data = get_frame(i);
			unlock(i);
			process_frame(i, frame_data);
		} else {
			unlock(i);
		}
	}
	cout << "ended checking kinect statuses" << endl;
}

int main(int argc, const char *argv[])
{
    pthread_t threads[NUM_KINECTS];
    int rc;
    int i;
    for (i = 0; i < NUM_KINECTS; i++) {
    	pthread_mutex_init(&(access_frame_mutexes[i]), NULL);
    	curr_frames[i].processed = true;
    	curr_frames[i].frame_data = -2;
        cout << "main() : creating thread, " << i << endl;
        rc = pthread_create(&threads[i], NULL, kinect_reader, (void *) i);
		if (rc) {
			cout << "Error: unable to create thread," << rc << endl;
		    exit(-1);
		}
    }
    
    while(true) {
    	sleep(1);
    	poll_kinect_statuses();
    }
    
    pthread_exit(NULL);
}
