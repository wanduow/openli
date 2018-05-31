/*
 *
 * Copyright (c) 2018 The University of Waikato, Hamilton, New Zealand.
 * All rights reserved.
 *
 * This file is part of OpenLI.
 *
 * This code has been developed by the University of Waikato WAND
 * research group. For further information please see http://www.wand.net.nz/
 *
 * OpenLI is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenLI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

#ifndef OPENLI_MEDIATOR_H_
#define OPENLI_MEDIATOR_H_

#include <libwandder.h>
#include <libwandder_etsili.h>
#include <libtrace/simple_circular_buffer.h>
#include <uthash.h>
#include "netcomms.h"
#include "export_buffer.h"

typedef struct med_epoll_ev {
    int fdtype;
    int fd;
    void *state;
} med_epoll_ev_t;

enum {
    MED_EPOLL_COLL_CONN,
    MED_EPOLL_PROVISIONER,
    MED_EPOLL_LEA,
    MED_EPOLL_COLLECTOR,
    MED_EPOLL_KA_TIMER,
    MED_EPOLL_KA_RESPONSE_TIMER,
    MED_EPOLL_SIGNAL,
    MED_EPOLL_SIGCHECK_TIMER,
};

typedef struct med_coll_state {
    net_buffer_t *incoming;
    int disabled;
} med_coll_state_t;

typedef struct handover {
    char *ipstr;
    char *portstr;
    int handover_type;
    med_epoll_ev_t *outev;
    med_epoll_ev_t *aliveev;
    med_epoll_ev_t *aliverespev;
} handover_t;

typedef struct med_agency_state {
    export_buffer_t buf;
    libtrace_scb_t *incoming;
    int outenabled;
    int failmsg;
    int main_fd;
    int katimer_fd;
    int karesptimer_fd;
    wandder_encoded_result_t *pending_ka;
    int64_t lastkaseq;
    wandder_encoder_t *encoder;
    wandder_etsispec_t *decoder;
    handover_t *parent;
} med_agency_state_t;

typedef struct mediator_collector {
    med_epoll_ev_t *colev;
} mediator_collector_t;

typedef struct mediator_provisioner {
    med_epoll_ev_t *provev;
    int sentinfo;
    net_buffer_t *outgoing;
    net_buffer_t *incoming;
} mediator_prov_t;

enum {
    HANDOVER_HI2 = 2,
    HANDOVER_HI3 = 3,
};

typedef struct liidmapping liid_map_t;

typedef struct mediator_agency {
    char *agencyid;
    int awaitingconfirm;
    int disabled;
    handover_t *hi2;
    handover_t *hi3;
    uint8_t karespreq;
} mediator_agency_t;

typedef struct med_state {
    uint32_t mediatorid;
    char *conffile;
    char *mediatorname;
    char *operatorid;
    char *listenaddr;
    char *listenport;

    char *provaddr;
    char *provport;

    uint32_t keepalivefreq;
    uint32_t keepalivewait;

    libtrace_list_t *collectors;
    libtrace_list_t *agencies;

    int epoll_fd;
    med_epoll_ev_t *listenerev;
    med_epoll_ev_t *signalev;
    med_epoll_ev_t *timerev;

    mediator_prov_t provisioner;

    liid_map_t *liids;

} mediator_state_t;

struct liidmapping {
    char *liid;
    mediator_agency_t *agency;
    UT_hash_handle hh;
};

#endif

// vim: set sw=4 tabstop=4 softtabstop=4 expandtab :
