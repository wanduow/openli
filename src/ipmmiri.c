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

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <libtrace.h>
#include <libwandder.h>
#include <libwandder_etsili.h>

#include "logger.h"
#include "collector.h"
#include "intercept.h"
#include "collector_export.h"
#include "etsili_core.h"
#include "ipmmiri.h"

int ipmm_iri(libtrace_packet_t *pkt, openli_export_recv_t *irimsg,
        voipintercept_t *vint, voipintshared_t *cin,
        etsili_iri_type_t iritype, uint8_t ipmmiri_style,
        shared_global_info_t *info) {

    irimsg->type = OPENLI_EXPORT_IPMMIRI;

    irimsg->destid = vint->common.destid;
    irimsg->data.ipmmiri.liid = strdup(vint->common.liid);
    irimsg->data.ipmmiri.packet = pkt;
    irimsg->data.ipmmiri.cin = cin->cin;
    irimsg->data.ipmmiri.iritype = iritype;
    irimsg->data.ipmmiri.ipmmiri_style = ipmmiri_style;
    irimsg->data.ipmmiri.colinfo = info;

}

int encode_ipmmiri(wandder_encoder_t **encoder, openli_ipmmiri_job_t *job,
        exporter_intercept_msg_t *intdetails, uint32_t seqno,
        openli_exportmsg_t *msg) {

    void *l3, *content, *transport;
    uint16_t ethertype;
    uint32_t rem;
    uint8_t proto;
    wandder_etsipshdr_data_t hdrdata;
    struct timeval tv;

    content = NULL;
    l3 = trace_get_layer3(job->packet, &ethertype, &rem);
    if (l3 == NULL || rem < sizeof(libtrace_ip_t)) {
        logger(LOG_DAEMON, "OpenLI: packet intended for IPMM IRI is invalid.");
        return -1;
    }

    transport = trace_get_transport(job->packet, &proto, &rem);
    if (transport) {
        if (proto == TRACE_IPPROTO_UDP) {
            content = trace_get_payload_from_udp((libtrace_udp_t *)transport,
                    &rem);
            if (rem == 0) {
                content = NULL;
            }
        }
    }

    if (*encoder == NULL) {
        *encoder = init_wandder_encoder();
    } else {
        reset_wandder_encoder(*encoder);
    }

    tv = trace_get_timeval(job->packet);

    hdrdata.liid = intdetails->liid;
    hdrdata.liid_len = intdetails->liid_len;
    hdrdata.authcc = intdetails->authcc;
    hdrdata.authcc_len = intdetails->authcc_len;
    hdrdata.delivcc = intdetails->delivcc;
    hdrdata.delivcc_len = intdetails->delivcc_len;
    hdrdata.operatorid = job->colinfo->operatorid;
    hdrdata.operatorid_len = job->colinfo->operatorid_len;
    hdrdata.networkelemid = job->colinfo->networkelemid;
    hdrdata.networkelemid_len = job->colinfo->networkelemid_len;
    hdrdata.intpointid = job->colinfo->intpointid;
    hdrdata.intpointid_len = job->colinfo->intpointid_len;

    memset(msg, 0, sizeof(openli_exportmsg_t));

    if (job->ipmmiri_style == OPENLI_IPMMIRI_ORIGINAL) {
        msg->msgbody = encode_etsi_ipmmiri(*encoder, &hdrdata,
                (int64_t)(job->cin), (int64_t)seqno, job->iritype, &tv, l3,
                rem);
        msg->ipcontents = (uint8_t *)l3;
        msg->ipclen = rem;
    } else if (job->ipmmiri_style == OPENLI_IPMMIRI_SIP) {
        if (content == NULL) {
            logger(LOG_DAEMON, "OpenLI: trying to create SIP IRI but packet has no SIP payload?");
            return -1;
        }

        msg->msgbody = encode_etsi_sipiri(*encoder, &hdrdata,
                (int64_t)(job->cin), (int64_t)seqno, job->iritype, &tv,
                l3, ethertype, content, rem);
        msg->ipcontents = (uint8_t *)content;
        msg->ipclen = rem;
    }
    /* TODO style == H323 */

    msg->encoder = *encoder;
    msg->header = construct_netcomm_protocol_header(msg->msgbody->len,
            OPENLI_PROTO_ETSI_IRI, 0, &(msg->hdrlen));

    return 0;
}




// vim: set sw=4 tabstop=4 softtabstop=4 expandtab :
