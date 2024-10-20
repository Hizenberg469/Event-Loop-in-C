#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "utils.h"
#include "rt.h"
#include "timerlib.h"
#include "stp_el.h"

rt_table_t*
rt_create_new_rt_table(char* name) {

    rt_table_t* rtable = calloc(1, sizeof(rt_table_t));
    strncpy(rtable->rt_table_name, name, 32);
    return rtable;
}

static void
rt_entry_exp_timer_cbk(Timer_t* timer, void* arg) {

    rt_table_t* rt_table;
    rt_table_entry_t* rt_table_entry = (rt_table_entry_t*)arg;

    rt_table = rt_table_entry->rt_table;

    /* Handover the deletion of routing table entry to event loop thread */
    el_stp_update_routing_table(rt_table, ROUTE_DELETE, rt_table_entry);
}


int /*0 on success, -1 on failure*/
rt_insert_new_entry(rt_table_t* rt,
    char* dest, char mask,
    char* gw, char* oif,
    int exp_timer_in_millisec) {

    rt_table_entry_t* rt_table_entry = rt_look_up_rt_table_entry(
        rt, dest, mask);

    if (rt_table_entry) {
        printf("Error : Entry already exist for key %s/%d\n", dest, mask);
        return -1;
    }

    rt_table_entry =
        calloc(1, sizeof(rt_table_entry_t));

    rt_table_entry->rt_table = rt;

    strncpy(rt_table_entry->dest, dest, 16);
    rt_table_entry->mask = mask;
    strncpy(rt_table_entry->gw, gw, 16);
    strncpy(rt_table_entry->oif, oif, 32);
    rt_table_entry->next = 0;
    rt_table_entry->prev = 0;
    time(&rt_table_entry->last_updated_time);

    if (exp_timer_in_millisec) {
        rt_table_entry->exp_timer = setup_timer(
            rt_entry_exp_timer_cbk,
            exp_timer_in_millisec, 0, 0,
            (void*)rt_table_entry, false);

        start_timer(rt_table_entry->exp_timer);
    }

    /* Now insert the new rt_table_entry at the beginnig of the
     * list*/
    if (!rt->head) {
        rt->head = rt_table_entry;
        rt->count++;
        return 0;
    }

    rt_table_entry->next = rt->head;
    rt_table_entry->prev = 0;
    rt->head->prev = rt_table_entry;
    rt->head = rt_table_entry;
    rt->count++;
    return 0;
}

int /*0 on success, -1 on failure*/
rt_delete_rt_entry(rt_table_t* rt,
    char* dest, char mask) {

    rt_table_entry_t* rt_table_entry = rt_look_up_rt_table_entry(
        rt, dest, mask);

    if (!rt_table_entry)
        return -1;

    if (rt_table_entry->exp_timer) {
        delete_timer(rt_table_entry->exp_timer);
        rt_table_entry->exp_timer = NULL;
    }

    /*Now delete it*/
    if (rt->head == rt_table_entry) {
        rt->head = rt_table_entry->next;
        if (rt->head)
            rt->head->prev = NULL;
        free(rt_table_entry);
        rt->count--;
        return 0;
    }

    if (!rt_table_entry->prev) {
        if (rt_table_entry->next) {
            rt_table_entry->next->prev = NULL;
            rt_table_entry->next = 0;
            rt->count--;
            free(rt_table_entry);
            return 0;
        }
        rt->count--;
        free(rt_table_entry);
        return 0;
    }
    if (!rt_table_entry->next) {
        rt_table_entry->prev->next = NULL;
        rt_table_entry->prev = NULL;
        rt->count--;
        free(rt_table_entry);
        return 0;
    }

    rt_table_entry->prev->next = rt_table_entry->next;
    rt_table_entry->next->prev = rt_table_entry->prev;
    rt_table_entry->prev = 0;
    rt_table_entry->next = 0;
    rt->count--;
    free(rt_table_entry);
    return 0;
}

int /*0 on success, -1 on failure*/
rt_update_rt_entry(rt_table_t* rt,
    char* dest, char mask,
    char* new_gw, char* new_oif) {

    rt_table_entry_t* rt_table_entry = rt_look_up_rt_table_entry(
        rt, dest, mask);

    if (!rt_table_entry)
        return -1;

    if (strncmp(rt_table_entry->dest, dest, 16) == 0 &&
        rt_table_entry->mask == mask &&
        strncmp(rt_table_entry->gw, new_gw, 16) == 0 &&
        strncmp(rt_table_entry->oif, new_oif, 32) == 0) {

        /* Refresh the timer */
        if (rt_table_entry->exp_timer) {
            restart_timer(rt_table_entry->exp_timer);
        }

        return -1;
    }

    strncpy(rt_table_entry->dest, dest, 16);
    rt_table_entry->mask = mask;
    strncpy(rt_table_entry->gw, new_gw, 16);
    strncpy(rt_table_entry->oif, new_oif, 32);
    time(&rt_table_entry->last_updated_time);

    /* Refresh the timer */
    if (rt_table_entry->exp_timer) {

        restart_timer(rt_table_entry->exp_timer);
    }

    return 0;
}

void
rt_display_rt_table(rt_table_t* rt) {

    int i = 1;
    rt_table_entry_t* rt_table_entry = NULL;
    time_t curr_time = time(NULL);
    unsigned int uptime_in_seconds = 0;

    printf("# count = %u\n", rt->count);

    for (rt_table_entry = rt->head; rt_table_entry;
        rt_table_entry = rt_table_entry->next) {

        uptime_in_seconds = (unsigned int)difftime(
            curr_time, rt_table_entry->last_updated_time);

        printf("%d. %-18s %-4d %-18s %-18s", i,
            rt_table_entry->dest,
            rt_table_entry->mask,
            rt_table_entry->gw,
            rt_table_entry->oif);

        printf("Last updated : %s  ", hrs_min_sec_format(uptime_in_seconds)),
            printf("Exp time : %lu\n",
                rt_table_entry->exp_timer ? \
                timer_get_time_remaining_in_mill_sec(rt_table_entry->exp_timer) : 0);
        i++;
    }
}

rt_table_entry_t*
rt_look_up_rt_table_entry(rt_table_t* rt,
    char* dest, char mask) {

    rt_table_entry_t* rt_table_entry = NULL;

    for (rt_table_entry = rt->head; rt_table_entry;
        rt_table_entry = rt_table_entry->next) {

        if (strncmp(rt_table_entry->dest, dest, 16) == 0 &&
            rt_table_entry->mask == mask)
            return rt_table_entry;
    }
    return NULL;
}