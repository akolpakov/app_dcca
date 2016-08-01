#include "app_dcca.h"

static struct {
    struct dict_object * Auth_Application_Id;
    struct dict_object * CC_Request_Type;
    struct dict_object * CC_Request_Number;
    struct dict_object * Origin_State_Id;
    struct dict_object * Event_Timestamp;
    struct dict_object * Multiple_Services_Credit_Control;
    struct dict_object * Granted_Service_Unit;
    struct dict_object * CC_Total_Octets;
    struct dict_object * Validity_Time;
    struct dict_object * Result_Code;
    struct dict_object * Rating_Group;
    struct dict_object * Used_Service_Unit;
    struct dict_object * CC_Input_Octets;
    struct dict_object * CC_Output_Octets;
    struct dict_object * Subscription_Id;
    struct dict_object * Subscription_Id_Type;
    struct dict_object * Subscription_Id_Data;
} dcca_dict;

int dcca_cb_read( struct msg ** msg, struct dict_object * avp, struct avp_hdr ** avp_dst )
{
    struct avp * a = NULL;
    CHECK_FCT( fd_msg_search_avp ( *msg, avp, &a) );
    if (a) {
        CHECK_FCT( fd_msg_avp_hdr( a, avp_dst )  );
    }

    return 0;
}

int dcca_cb_put_value(struct msg * msg, struct dict_object * avp, union avp_value * value)
{
    struct avp * a = NULL;

    CHECK_FCT( fd_msg_avp_new ( avp, 0, &a ) );
    CHECK_FCT( fd_msg_avp_setvalue( a, value ) );
    CHECK_FCT( fd_msg_avp_add( msg, MSG_BRW_LAST_CHILD, a ) );

    return 0;
}

int dcca_cb_put_value_to_avp(struct avp * msg, struct dict_object * avp, union avp_value * value)
{
    struct avp * a = NULL;

    CHECK_FCT( fd_msg_avp_new ( avp, 0, &a ) );
    CHECK_FCT( fd_msg_avp_setvalue( a, value ) );
    CHECK_FCT( fd_msg_avp_add( msg, MSG_BRW_LAST_CHILD, a ) );

    return 0;
}

int dcca_cb_put(struct msg * msg, struct dict_object * avp, struct avp_hdr * avp_src)
{
    if (avp_src) {
        dcca_cb_put_value(msg, avp, avp_src->avp_value) ;
    }

    return 0;
}

// TODO: read all AVPs from message
//int dcca_read_avps_from_message(struct msg_avp * msg)
//{
//    struct avp * browse_avp = NULL;
//    struct avp_hdr * browse_avp_hdr = NULL;
//    struct dict_object * avp_model = NULL;
//    struct dict_avp_data avp_model_data;
//    struct dict_avp_data * avp_model_data_ptr = NULL;
//
//    CHECK_FCT_DO(  fd_msg_browse ( msg, MSG_BRW_FIRST_CHILD, &browse_avp, NULL ), browse_avp = NULL );
//    while (browse_avp) {
//        CHECK_FCT( fd_msg_model(browse_avp, &avp_model) );
//
//        if (avp_model) {
//            CHECK_FCT( fd_dict_getval(avp_model, &avp_model_data) );
//            CHECK_FCT( fd_msg_avp_hdr( browse_avp, &browse_avp_hdr ) );
//
//            avp_model_data_ptr = &avp_model_data;
//
//            unsigned char * buf = NULL; size_t buflen;
//
//            if (avp_model_data_ptr && (avp_model_data_ptr->avp_basetype == AVP_TYPE_GROUPED)) {
//                LOG_N("%s(%i|%i) Grouped", avp_model_data.avp_name, browse_avp_hdr->avp_vendor, browse_avp_hdr->avp_code);
//                struct avp * inavp = NULL;
//                CHECK_FCT_DO(  fd_msg_browse ( browse_avp, MSG_BRW_FIRST_CHILD, &inavp, NULL ), inavp = NULL );
//                while (inavp) {
//                    CHECK_FCT( dcca_read_avps_from_message(inavp) );
//                    struct avp * nextinavp = NULL;
//                    CHECK_FCT_DO(  fd_msg_browse ( inavp, MSG_BRW_NEXT, &nextinavp, NULL ), inavp = NULL  );
//                    inavp = nextinavp;
//                };
//
//            } else {
//                if (browse_avp_hdr->avp_value) {
//                    CHECK_MALLOC_DO(fd_dict_dump_avp_value(&buf, &buflen, NULL, browse_avp_hdr->avp_value, avp_model, 0, 0), return NULL);
//                } else {
//                    CHECK_MALLOC_DO(fd_dump_extend(&buf, &buflen, NULL, "(not set)"), return NULL);
//                }
//                LOG_N("%s(%i|%i) = %s", avp_model_data.avp_name, browse_avp_hdr->avp_vendor, browse_avp_hdr->avp_code, buf);
//            }
//
//        }
//
//        struct avp * nextavp = NULL;
//        CHECK_FCT_DO(  fd_msg_browse ( browse_avp, MSG_BRW_NEXT, &nextavp, NULL ), nextavp = NULL  );
//        browse_avp = nextavp;
//    };
//
//    return 0;
//}


int avp_search_child ( struct avp * msg, struct dict_object * what, struct avp ** avp )
{
	struct avp * nextavp;
	struct dict_avp_data 	dictdata;
	struct avp_hdr *avp_dst;

	TRACE_ENTRY("%p %p %p", msg, what, avp);

	*avp = NULL;

	CHECK_FCT(  fd_dict_getval(what, &dictdata)  );

	/* Loop on all top AVPs */
	CHECK_FCT(  fd_msg_browse(msg, MSG_BRW_FIRST_CHILD, (void *)&nextavp, NULL)  );
	while (nextavp) {
	    CHECK_FCT( fd_msg_avp_hdr( nextavp, &avp_dst )  );

		if ( (avp_dst->avp_code == dictdata.avp_code) && (avp_dst->avp_vendor == dictdata.avp_vendor) ) {
		    break;
		}

		/* Otherwise move to next AVP in the message */
		CHECK_FCT( fd_msg_browse(nextavp, MSG_BRW_NEXT, (void *)&nextavp, NULL) );
	}

	if (avp)
		*avp = nextavp;

	if (avp && nextavp) {
		struct dictionary * dict;
		CHECK_FCT( fd_dict_getdict( what, &dict) );
		CHECK_FCT_DO( fd_msg_parse_dict( nextavp, dict, NULL ), /* nothing */ );
	}

	if (avp || nextavp)
		return 0;
	else
		return ENOENT;
}

int get_imsi(struct msg * msg, struct avp ** avp)
{
    struct avp * nextavp = NULL, * a = NULL;
    struct avp_hdr * a_hdr = NULL;
    struct dict_avp_data dictdata;
	struct avp_hdr *avp_dst;

	*avp = NULL;

	CHECK_FCT(  fd_dict_getval(dcca_dict.Subscription_Id, &dictdata)  );

    CHECK_FCT(  fd_msg_browse(msg, MSG_BRW_FIRST_CHILD, (void *)&nextavp, NULL)  );
	while (nextavp) {
	    CHECK_FCT( fd_msg_avp_hdr( nextavp, &avp_dst )  );

		if ( (avp_dst->avp_code == dictdata.avp_code) && (avp_dst->avp_vendor == dictdata.avp_vendor) ) {
		    CHECK_FCT( avp_search_child ( nextavp, dcca_dict.Subscription_Id_Type, &a) );
		    if(a) {
		        CHECK_FCT( fd_msg_avp_hdr( a, &a_hdr )  );
		        if(a_hdr->avp_value->i32 == 1) {
		            CHECK_FCT( avp_search_child ( nextavp, dcca_dict.Subscription_Id_Data, avp) );
		            break;
		        }
		    }
		}

		/* Otherwise move to next AVP in the message */
		CHECK_FCT( fd_msg_browse(nextavp, MSG_BRW_NEXT, (void *)&nextavp, NULL) );
	}

	return 0;

}


// Processing application request. Called each Credit-Control-Request

static int dcca_cb( struct msg ** msg, struct avp * avp, struct session * sess, void * opaque, enum disp_action * act)
{
    struct msg * m;
    struct avp_hdr * val_app_id = NULL, * val_rt = NULL, * val_rn = NULL, * val_io = NULL, * val_oo = NULL, *val_imsi = NULL;
    struct avp *groupedavp = NULL, *groupedavp2 = NULL;
    union avp_value validity_time, total_octets, result_code, rating_group;
    struct avp * a = NULL, * aa = NULL;
    int input_octets = 0, output_octets = 0;
    char* imsi = NULL;

    LOG_N("CCR processor");

    TRACE_ENTRY("%p %p %p %p", msg, avp, sess, act);

    if (msg == NULL)
        return EINVAL;

//    // Read AVPs from request
//
//    CHECK_FCT( dcca_read_avps_from_message(*msg) );


    // Read AVPs

    CHECK_FCT( dcca_cb_read(msg, dcca_dict.Auth_Application_Id, &val_app_id) );
    CHECK_FCT( dcca_cb_read(msg, dcca_dict.CC_Request_Type, &val_rt) );
    CHECK_FCT( dcca_cb_read(msg, dcca_dict.CC_Request_Number, &val_rn) );

    // Read IMSI

    CHECK_FCT( get_imsi(*msg, &a) );
    if (a) {
        CHECK_FCT( fd_msg_avp_hdr( a, &val_imsi )  );
        int len = val_imsi->avp_value->os.len;
        imsi = malloc(len * sizeof(char));

        int i;
        for (i = 0; i < len; i++) {
            imsi[i] = val_imsi->avp_value->os.data[i];
        }
        imsi[len] = 0;
    }

    // Read Input / Output Octets

    CHECK_FCT( fd_msg_search_avp ( *msg, dcca_dict.Multiple_Services_Credit_Control, &a) );
    if (a) {
        CHECK_FCT( avp_search_child ( a, dcca_dict.Used_Service_Unit, &a) );
        if (a) {
            CHECK_FCT( avp_search_child ( a, dcca_dict.CC_Input_Octets, &aa) );
            if (aa) {
                CHECK_FCT( fd_msg_avp_hdr( aa, &val_io )  );
                input_octets = val_io->avp_value->i32;
            }
            CHECK_FCT( avp_search_child ( a, dcca_dict.CC_Output_Octets, &aa) );
            if (aa) {
                CHECK_FCT( fd_msg_avp_hdr( aa, &val_oo )  );
                output_octets = val_oo->avp_value->i32;
            }
        }
    }

    LOG_N("IMSI: %s", imsi);
    LOG_N("IN: %i", input_octets);
    LOG_N("OUT: %i", output_octets);

    // Create response message

    CHECK_FCT( fd_msg_new_answer_from_req ( fd_g_config->cnf_dict, msg, 0 ) );
    m = *msg;
    CHECK_FCT( fd_msg_rescode_set( m, "DIAMETER_SUCCESS", NULL, NULL, 1 ) );

    // Put params to response

    CHECK_FCT( dcca_cb_put(m, dcca_dict.Auth_Application_Id, val_app_id) );
    CHECK_FCT( dcca_cb_put(m, dcca_dict.CC_Request_Type, val_rt) );
    CHECK_FCT( dcca_cb_put(m, dcca_dict.CC_Request_Number, val_rn) );

    // Multiple Services CC Group

    total_octets.i64 = rest_api_vsca(imsi, input_octets, output_octets);
    CHECK_FCT( fd_msg_avp_new ( dcca_dict.Multiple_Services_Credit_Control, 0, &groupedavp ) );

    // Granted Service Unit

    CHECK_FCT( fd_msg_avp_new ( dcca_dict.Granted_Service_Unit, 0, &groupedavp2 ) );
    CHECK_FCT( dcca_cb_put_value_to_avp(groupedavp2, dcca_dict.CC_Total_Octets, &total_octets) );
    CHECK_FCT( fd_msg_avp_add( groupedavp, MSG_BRW_LAST_CHILD, groupedavp2 ) );

    // Result code

    result_code.i32 = 2001;
    CHECK_FCT( dcca_cb_put_value_to_avp(groupedavp, dcca_dict.Result_Code, &result_code) );

    // Rating group

    rating_group.i32 = 1001;
    CHECK_FCT( dcca_cb_put_value_to_avp(groupedavp, dcca_dict.Rating_Group, &rating_group) );

    // Validity Time

    validity_time.i32 = 60;
    CHECK_FCT( dcca_cb_put_value_to_avp(groupedavp, dcca_dict.Validity_Time, &validity_time) );

    CHECK_FCT( fd_msg_avp_add( m, MSG_BRW_LAST_CHILD, groupedavp ) );

    // Send the answer

    *act = DISP_ACT_SEND;

    return 0;
}

// Extension entry point. Called when application starts

static int dcca_entry(char * conffile)
{
    struct disp_when data;

    TRACE_ENTRY("%p", conffile);

    // get AVPs from dict by name
    // TODO: temporary, needs to be done more universal

    CHECK_FCT( fd_dict_search( fd_g_config->cnf_dict, DICT_AVP, AVP_BY_NAME, "Auth-Application-Id", &dcca_dict.Auth_Application_Id, ENOENT) );
    CHECK_FCT( fd_dict_search( fd_g_config->cnf_dict, DICT_AVP, AVP_BY_NAME, "CC-Request-Type", &dcca_dict.CC_Request_Type, ENOENT) );
    CHECK_FCT( fd_dict_search( fd_g_config->cnf_dict, DICT_AVP, AVP_BY_NAME, "CC-Request-Number", &dcca_dict.CC_Request_Number, ENOENT) );
    CHECK_FCT( fd_dict_search( fd_g_config->cnf_dict, DICT_AVP, AVP_BY_NAME, "Origin-State-Id", &dcca_dict.Origin_State_Id, ENOENT) );
    CHECK_FCT( fd_dict_search( fd_g_config->cnf_dict, DICT_AVP, AVP_BY_NAME, "Event-Timestamp", &dcca_dict.Event_Timestamp, ENOENT) );
    CHECK_FCT( fd_dict_search( fd_g_config->cnf_dict, DICT_AVP, AVP_BY_NAME, "Multiple-Services-Credit-Control", &dcca_dict.Multiple_Services_Credit_Control, ENOENT) );
    CHECK_FCT( fd_dict_search( fd_g_config->cnf_dict, DICT_AVP, AVP_BY_NAME, "Granted-Service-Unit", &dcca_dict.Granted_Service_Unit, ENOENT) );
    CHECK_FCT( fd_dict_search( fd_g_config->cnf_dict, DICT_AVP, AVP_BY_NAME, "CC-Total-Octets", &dcca_dict.CC_Total_Octets, ENOENT) );
    CHECK_FCT( fd_dict_search( fd_g_config->cnf_dict, DICT_AVP, AVP_BY_NAME, "Validity-Time", &dcca_dict.Validity_Time, ENOENT) );
    CHECK_FCT( fd_dict_search( fd_g_config->cnf_dict, DICT_AVP, AVP_BY_NAME, "Result-Code", &dcca_dict.Result_Code, ENOENT) );
    CHECK_FCT( fd_dict_search( fd_g_config->cnf_dict, DICT_AVP, AVP_BY_NAME, "Rating-Group", &dcca_dict.Rating_Group, ENOENT) );
    CHECK_FCT( fd_dict_search( fd_g_config->cnf_dict, DICT_AVP, AVP_BY_NAME, "Used-Service-Unit", &dcca_dict.Used_Service_Unit, ENOENT) );
    CHECK_FCT( fd_dict_search( fd_g_config->cnf_dict, DICT_AVP, AVP_BY_NAME, "CC-Input-Octets", &dcca_dict.CC_Input_Octets, ENOENT) );
    CHECK_FCT( fd_dict_search( fd_g_config->cnf_dict, DICT_AVP, AVP_BY_NAME, "CC-Output-Octets", &dcca_dict.CC_Output_Octets, ENOENT) );
    CHECK_FCT( fd_dict_search( fd_g_config->cnf_dict, DICT_AVP, AVP_BY_NAME, "Subscription-Id", &dcca_dict.Subscription_Id, ENOENT) );
    CHECK_FCT( fd_dict_search( fd_g_config->cnf_dict, DICT_AVP, AVP_BY_NAME, "Subscription-Id-Type", &dcca_dict.Subscription_Id_Type, ENOENT) );
    CHECK_FCT( fd_dict_search( fd_g_config->cnf_dict, DICT_AVP, AVP_BY_NAME, "Subscription-Id-Data", &dcca_dict.Subscription_Id_Data, ENOENT) );

    // Register the dispatch callbacks

    memset(&data, 0, sizeof(data));
    CHECK_FCT( fd_dict_search( fd_g_config->cnf_dict, DICT_APPLICATION, APPLICATION_BY_NAME, "Diameter Credit Control Application", &data.app, ENOENT) );
    CHECK_FCT( fd_dict_search( fd_g_config->cnf_dict, DICT_COMMAND, CMD_BY_NAME, "Credit-Control-Request", &data.command, ENOENT) );
    CHECK_FCT( fd_disp_register( dcca_cb, DISP_HOW_CC, &data, NULL, NULL ) );

    // Advertise the support for the Diameter Base Accounting application in the peer

    CHECK_FCT( fd_disp_app_support ( data.app, NULL, 0, 1 ) );

    return 0;
}

EXTENSION_ENTRY("app_dcca", dcca_entry, "dict_dcca");
