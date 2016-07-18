#include "app_dcca.h"

static int dcca_cb( struct msg ** msg, struct avp * avp, struct session * sess, void * opaque, enum disp_action * act)
{
        struct msg * m;

        LOG_N("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! CB");

        TRACE_ENTRY("%p %p %p %p", msg, avp, sess, act);


        /* Create the answer message */
        CHECK_FCT( fd_msg_new_answer_from_req ( fd_g_config->cnf_dict, msg, 0 ) );
        m = *msg;

        /* Set the Origin-Host, Origin-Realm, Result-Code AVPs */
        CHECK_FCT( fd_msg_rescode_set( m, "DIAMETER_SUCCESS", NULL, NULL, 1 ) );

        /* Send the answer */
        *act = DISP_ACT_SEND;

        return 0;
}

static int dcca_entry(char * conffile)
{
        struct disp_when data;

        TRACE_ENTRY("%p", conffile);

        /* Register the dispatch callbacks */
        memset(&data, 0, sizeof(data));
        CHECK_FCT( fd_dict_search( fd_g_config->cnf_dict, DICT_APPLICATION, APPLICATION_BY_NAME, "Diameter Credit Control Application", &data.app, ENOENT) );
        CHECK_FCT( fd_dict_search( fd_g_config->cnf_dict, DICT_COMMAND, CMD_BY_NAME, "Credit-Control-Request", &data.command, ENOENT) );
        CHECK_FCT( fd_disp_register( dcca_cb, DISP_HOW_CC, &data, NULL, NULL ) );

        /* Advertise the support for the Diameter Base Accounting application in the peer */
        CHECK_FCT( fd_disp_app_support ( data.app, NULL, 0, 1 ) );

        return 0;
}

EXTENSION_ENTRY("app_dcca", dcca_entry, "dict_dcca");
