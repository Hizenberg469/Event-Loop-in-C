#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "event_loop.h"

event_loop_t el;
static int upload = 0;
static int download = 0;

static EL_RES_T
upload_fn(void* arg) {

    while (upload < 100) {

        upload += 2;
        //sleep(4);
        printf("upload percent-age = %d\n", upload);
        if (upload % 10 == 0 && upload != 100) {

            return EL_CONTINUE;
           /* task_create_new_job(&el, upload_fn, NULL);
            return ;*/
        }
    }
    return EL_FINISH;
}

static EL_RES_T
download_fn(void* arg) {

    while (download < 100) {

        download += 2;
        printf("download percent-age = %d\n", download);
        if (download % 10 == 0 && download != 100) {

            return EL_CONTINUE;
            /*task_create_new_job(&el, download_fn, NULL);
            return;*/
        }
    }
    return EL_FINISH;
}

int
main(int argc, char** argv) {

    event_loop_init(&el);
    event_loop_run(&el);
    sleep(1);

    task_create_new_job(&el, upload_fn, NULL, TASK_PRIORITY_LOW);
    task_create_new_job(&el, download_fn, NULL, TASK_PRIORITY_HIGH);

    printf("End of main\n");
    scanf("\n");
    return 0;
}