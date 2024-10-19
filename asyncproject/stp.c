#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "network_utils.h"
#include "rt.h"
#include "stp_el.h"


extern event_loop_t el;
rt_table_t* rt_table = NULL;

int
stp_update_routing_table(rt_table_t* rt_table, uint32_t cmd_code, rt_table_entry_t* rt_entry) {

    int rc;
    printf("   Code = %u Route update recvd  : %s\n", cmd_code, rt_entry->dest);
    switch (cmd_code) {
    case  ROUTE_CREATE:
        rc = rt_insert_new_entry(rt_table, rt_entry->dest, 32, rt_entry->gw, rt_entry->oif, rt_entry->exp_timer_msec);
        break;
    case ROUTE_UPDATE:
        rc = rt_update_rt_entry(rt_table, rt_entry->dest, 32, rt_entry->gw, rt_entry->oif);
        break;
    case ROUTE_DELETE:
        rc = rt_delete_rt_entry(rt_table, rt_entry->dest, 32);
        break;
    default:;
    }
    return rc;
}

static void
pkt_process_fn(char* msg_recvd, uint32_t msg_size, char* sender_ip, uint32_t port_no, uint32_t fd) {

    printf("route recvd on port no %d from IP %s\n", port_no, sender_ip);
    uint32_t* cmd_code = (uint32_t*)msg_recvd;
    rt_table_entry_t* rt_entry = (rt_table_entry_t*)(cmd_code + 1);
    rt_entry->exp_timer_msec = RT_ENTRY_EXP_TIMER * 1000;

    el_stp_update_routing_table(rt_table, *cmd_code, rt_entry);
}

static void
cli_handler(int choice)
{
    switch (choice)
    {
    case 1:
        //rt_display_rt_table(rt_table);

        rt_display_rt_table_preemption_context_save(rt_table);
        printf("\n\n");
        break;
    case 2:
    {
        rt_table_entry_t rt_entry;
        printf("Enter Dest Address : ");
        scanf("%s", rt_entry.dest);
        printf("Enter Gateway  Address : ");
        scanf("%s", rt_entry.gw);
        printf("Enter OIF name : ");
        scanf("%s", rt_entry.oif);
        el_stp_update_routing_table(rt_table, ROUTE_CREATE, &rt_entry);
    }
    break;
    case 3:
        // do self
        break;
    case 4:
    {
        rt_table_entry_t rt_entry;
        printf("Enter Dest Address : ");
        scanf("%s", rt_entry.dest);
        if (el_stp_update_routing_table(rt_table, ROUTE_DELETE, &rt_entry))
        {
            printf("No Such entry\n");
        }
    }
    break;
    case 5:
    {
        int portno;
        printf("Listening port no ? ");
        scanf("%d", &portno);
        udp_server_create_and_start(
            "127.0.0.1", portno, pkt_process_fn);
    }
    break;
    case 6:
    {
        rt_table_entry_t rt_entry_template;
        printf("Entry Dest Address : ");
        scanf("%s", rt_entry_template.dest);
        el_stp_serialize_and_send_rt_entry(rt_table, &rt_entry_template);
    }
    break;
    case 7:
    {
        el_stp_delete_rt_table(rt_table);
    }
    break;
    case 8:
        exit(0);
    default:
        break;
    }
}

int
main(int argc, char** argv) {

    if (!rt_table) {
        rt_table =
            rt_create_new_rt_table("Table1");
        printf(" New Routing Table Created\n");
    }

    for (;;) {

        printf("Main Menu\n");
        printf("\t 1.  : Display Routing Table\n");
        printf("\t 2.  : Create New RT Entry\n");
        printf("\t 3.  : Update Existing RT Entry\n");
        printf("\t 4.  : Delete RT entry\n");
        printf("\t 5.  : Start Pkt Listener thread\n");
        printf("\t 6.  : Serialize and send RT entry\n");
        printf("\t 7.  : Delete RT Table\n");
        printf("\t 8.  : exit\n");

        int choice;
        printf("Enter Choice : ");
        scanf("%d", &choice);

        cli_handler(choice);
    }
    return 0;
}