#ifndef __RT__
#define __RT__

/*Opaque Data structures*/
typedef struct rt_table_ rt_table_t;
typedef struct rt_table_entry_ rt_table_entry_t;
typedef struct Timer_ Timer_t;


#define RT_ENTRY_EXP_TIMER 30  // 30 sec

#define ROUTE_CREATE    1
#define ROUTE_UPDATE    2
#define ROUTE_DELETE    3

struct rt_table_entry_ {

    char dest[16];
    char mask;
    char gw[16];
    char oif[32];
    time_t last_updated_time;
    struct rt_table_entry_* next;
    struct rt_table_entry_* prev;
    rt_table_t* rt_table; /* back ptr to owning rt table*/

    Timer_t* exp_timer;
    int exp_timer_msec;
};

struct rt_table_ {

    char rt_table_name[32];
    struct rt_table_entry_* head;
    uint32_t count;
};

rt_table_t*
rt_create_new_rt_table(char* name);

int /*0 on success, -1 on failure*/
rt_insert_new_entry(rt_table_t* rt,
    char* dest, char mask,
    char* gw, char* oif,
    int exp_timer_in_millisec);

int /*0 on success, -1 on failure*/
rt_delete_rt_entry(rt_table_t* rt,
    char* dest, char mask);

int /*0 on success, -1 on failure*/
rt_update_rt_entry(rt_table_t* rt,
    char* dest, char mask,
    char* new_gw, char* new_oif);

void
rt_display_rt_table(rt_table_t* rt);

rt_table_entry_t*
rt_look_up_rt_table_entry(rt_table_t* rt,
    char* dest, char mask);

#endif /* __RT__ */