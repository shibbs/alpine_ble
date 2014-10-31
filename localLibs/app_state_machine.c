/**
State Machine framework for BLE Michron ble

Rev 1.0
Stephen Hibbs for Alpine Labs LLC

*/

/**
Notes on Creating a service 
from   - https://devzone.nordicsemi.com/question/12768/custom-ble-service-help/ : 
 In order to create a service, basically, u need to have 2 basic structures (one is for init, 
 the other one for storing necessery info about the service itself). Take battery service (ble_bas)
 as an example. We have ble_bas_s for the service itself and ble_bas_init_t for passing
 initialization info to the custom service.

Then you will need to create an init func in which you will add your custom characteristics (
look at ble_bas_init function in ble_bas.c). In your case, you will need to add 3 characteristics.
In order to to add characteristic, you will need ble_gatts_char_md_t (for characteristic metadata)
, ble_gatts_attr_t (for characteristic value, UUID, etc...) and ble_gatts_attr_md_t
(for setting access permission to characteristic). Additionally, you need to set up and 
'send' function in your custom service as well. This will take care of sending characteristic
data to peripheral device (take a look at ble_hrs_heart_rate_measurement_send in ble_hrs.c for reference).

Considerting initialization of advdata, the template ble_ble_template mentioned in the AN
already did the basic initialization. you can play with the config if you want to.
*/

#include "alpine_includes.h"
#include "app_state_machine.h"
#include "alpine_tl_state_machine.h"
#include <string.h>
#include "nordic_common.h"
//#include "ble_l2cap.h"
#include "ble_srv_common.h"
#include "app_util.h"


/**@brief Function for handling the Connect event.
 *
 * @param[in]   p_ble_sm       ble SM Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_connect(ble_sm_t * p_ble_sm, ble_evt_t * p_ble_evt)
{
    p_ble_sm->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}


/**@brief Function for handling the Disconnect event.
 *
 * @param[in]   p_ble_sm    ble SM Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_disconnect(ble_sm_t * p_ble_sm, ble_evt_t * p_ble_evt)
{
    UNUSED_PARAMETER(p_ble_evt);
    p_ble_sm->conn_handle = BLE_CONN_HANDLE_INVALID;
}



/**@brief Function for handling the Write event.
 *
 * @param[in]   p_ble_sm    ble SM Service structure.
 * @param[in]   p_ble_evt   Event received from the BLE stack.
 */
static void on_write(ble_sm_t * p_ble_sm, ble_evt_t * p_ble_evt)
{
	ble_gatts_evt_write_t * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
	
	//check if it is a shutter write event
	if ((p_evt_write->handle == p_ble_sm->ble_sm_shutter_char_handles.value_handle) &&
			//	(p_evt_write->len == 1) && FLAG SAH removed as we do not yet know packet size to use
				(p_ble_sm->shutter_write_handler != NULL))
	{
		//This has been altered to now accept an array
			p_ble_sm->shutter_write_handler(p_ble_sm,  p_evt_write->data);
	}
	
	//check for timelapse packet write event
	if ((p_evt_write->handle == p_ble_sm->ble_sm_tl_pkt_char_handles.value_handle) &&
			//	(p_evt_write->len == 1) && FLAG SAH removed as we do not yet know packet size to use
				(p_ble_sm->tl_pkt_write_handler != NULL))
	{
			p_ble_sm->tl_pkt_write_handler(p_ble_sm, p_evt_write->data);
	}

}


/**@brief Function for adding the App SM state characteristic.
 *
 * @param[in]   p_app_sm        app state machine structure.
 * @param[in]   p_app_sm_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t sm_state_char_add(ble_sm_t * p_ble_sm, const ble_sm_init_t * p_ble_sm_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
	  uint8_t             initial_ble_sm_state;
//    uint8_t             encoded_report_ref[BLE_SRV_ENCODED_REPORT_REF_LEN];
 //   uint8_t             init_len;

    // Add Battery Level characteristic
    if (true )
    {
        memset(&cccd_md, 0, sizeof(cccd_md));

        // According to our ble SM spec, the read operation on cccd should be possible without
        // authentication.
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
				BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
       // cccd_md.write_perm = p_ble_sm_init->ble_sm_char_attr_md.cccd_write_perm;
        cccd_md.vloc       = BLE_GATTS_VLOC_STACK;
    }

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.notify = 1; // (p_ble_sm->is_notification_supported) ? 1 : 0;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md; //(p_ble_sm->is_notification_supported) ? &cccd_md : NULL;
    char_md.p_sccd_md         = NULL;

		ble_uuid.type = p_ble_sm->uuid_type;
		ble_uuid.uuid = BLE_SM_UUID_STATE_CHAR;
				
		memset(&attr_md, 0, sizeof(attr_md));
		BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
		BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    
		attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    initial_ble_sm_state = p_ble_sm_init->initial_ble_sm_state;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(uint8_t);
    attr_char_value.p_value   = &initial_ble_sm_state;

    err_code = sd_ble_gatts_characteristic_add(p_ble_sm->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_ble_sm->ble_sm_state_char_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    else
    {
        p_ble_sm->report_ref_handle = BLE_GATT_HANDLE_INVALID;
    }

    return NRF_SUCCESS; 
		
}

/**@brief Function for adding the App SM time characteristic.
 *
 * @param[in]   p_app_sm        app state machine structure.
 * @param[in]   p_app_sm_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t sm_time_char_add(ble_sm_t * p_ble_sm, const ble_sm_init_t * p_ble_sm_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
	  uint8_t             initial_ble_sm_time;
//    uint8_t             encoded_report_ref[BLE_SRV_ENCODED_REPORT_REF_LEN];
 //   uint8_t             init_len;

    // Add read characteristic
    if (true )
    {
        memset(&cccd_md, 0, sizeof(cccd_md));

        // According to our ble SM spec, the read operation on cccd should be possible without
        // authentication.
        BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
				BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
      //  cccd_md.write_perm = p_ble_sm_init->ble_sm_char_attr_md.cccd_write_perm;
        cccd_md.vloc       = BLE_GATTS_VLOC_STACK;
    }

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read   = 1;
    char_md.char_props.notify = 1; // (p_ble_sm->is_notification_supported) ? 1 : 0;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md; //(p_ble_sm->is_notification_supported) ? &cccd_md : NULL;
    char_md.p_sccd_md         = NULL;

		ble_uuid.type = p_ble_sm->uuid_type;
		ble_uuid.uuid = BLE_SM_UUID_TIME_CHAR;
				
		memset(&attr_md, 0, sizeof(attr_md));
		BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
		BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);
    
		attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    initial_ble_sm_time = p_ble_sm_init->initial_ble_sm_time;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(uint8_t);
    attr_char_value.p_value   = &initial_ble_sm_time; 

    err_code = sd_ble_gatts_characteristic_add(p_ble_sm->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_ble_sm->ble_sm_time_char_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    else
    {
        p_ble_sm->report_ref_handle = BLE_GATT_HANDLE_INVALID;
    }

    return NRF_SUCCESS; 
		
}

/**@brief Function for adding the App SM shutter characteristic.
 *
 * @param[in]   p_app_sm        app state machine structure.
 * @param[in]   p_app_sm_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
/**@brief Function for adding the App SM timelapse packets characteristic.
 *
 * @param[in]   p_app_sm        app state machine structure.
 * @param[in]   p_app_sm_init   Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
static uint32_t sm_tl_pkt_char_add(ble_sm_t * p_ble_sm, const ble_sm_init_t * p_ble_sm_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
 //   ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
	  uint8_t             initial_ble_sm_state;
//    uint8_t             encoded_report_ref[BLE_SRV_ENCODED_REPORT_REF_LEN];
 //   uint8_t             init_len;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.write   = 1;
    char_md.char_props.notify = 1; // (p_ble_sm->is_notification_supported) ? 1 : 0;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
  //  char_md.p_cccd_md         = &cccd_md; //(p_ble_sm->is_notification_supported) ? &cccd_md : NULL;
    char_md.p_sccd_md         = NULL;

		ble_uuid.type = p_ble_sm->uuid_type;
		ble_uuid.uuid = BLE_SM_UUID_TL_PKT_CHAR;
				
		memset(&attr_md, 0, sizeof(attr_md));
		BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
		BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    
		attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    initial_ble_sm_state = p_ble_sm_init->initial_ble_sm_state;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = MAX_BLE_PACKET_LEN*sizeof(uint8_t);
    attr_char_value.p_value   = &initial_ble_sm_state;

    err_code = sd_ble_gatts_characteristic_add(p_ble_sm->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_ble_sm->ble_sm_tl_pkt_char_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    else
    {
        p_ble_sm->report_ref_handle = BLE_GATT_HANDLE_INVALID;
    }

    return NRF_SUCCESS; 
		
}



static uint32_t sm_shutter_char_add(ble_sm_t * p_ble_sm, const ble_sm_init_t * p_ble_sm_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
 //   ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
	  uint8_t             initial_ble_sm_state;
//    uint8_t             encoded_report_ref[BLE_SRV_ENCODED_REPORT_REF_LEN];
//    uint8_t             init_len; //this is the packet length


    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.write   = 1;
    char_md.char_props.notify = 1; // (p_ble_sm->is_notification_supported) ? 1 : 0;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
  //  char_md.p_cccd_md         = &cccd_md; //(p_ble_sm->is_notification_supported) ? &cccd_md : NULL;
    char_md.p_sccd_md         = NULL;

		ble_uuid.type = p_ble_sm->uuid_type;
		ble_uuid.uuid = BLE_SM_UUID_SHUTTER_CHAR;
				
		memset(&attr_md, 0, sizeof(attr_md));
		BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
		BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);
    
		attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;

    initial_ble_sm_state = p_ble_sm_init->initial_ble_sm_state;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t); //initiate the length of this packet rcv
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = MAX_BLE_PACKET_LEN * sizeof(uint8_t);
    attr_char_value.p_value   = &initial_ble_sm_state;

    err_code = sd_ble_gatts_characteristic_add(p_ble_sm->service_handle, &char_md,
                                               &attr_char_value,
                                               &p_ble_sm->ble_sm_shutter_char_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    else
    {
        p_ble_sm->report_ref_handle = BLE_GATT_HANDLE_INVALID;
    }

    return NRF_SUCCESS; 
		
}



void ble_sm_on_ble_evt(ble_sm_t * p_ble_sm, ble_evt_t * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_ble_sm, p_ble_evt);
            break;

        case BLE_GAP_EVT_DISCONNECTED:
            on_disconnect(p_ble_sm, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_ble_sm, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}




//This is the services initialization method
uint32_t ble_sm_init(ble_sm_t * p_ble_sm, const ble_sm_init_t * p_ble_sm_init)
{
    uint32_t   err_code;
    ble_uuid_t ble_uuid;

    // Initialize service structure
    p_ble_sm->shutter_write_handler  = p_ble_sm_init->shutter_write_handler;
	  p_ble_sm->tl_pkt_write_handler  = p_ble_sm_init->tl_pkt_write_handler;
    p_ble_sm->conn_handle               = BLE_CONN_HANDLE_INVALID;
//   p_ble_sm->is_notification_supported = p_ble_sm_init->support_notification;
//    p_ble_sm->ble_sm_state_last        = INVALID_BATTERY_LEVEL;

    // Add service
		//FLAG SAH
 //   BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_BATTERY_SERVICE);

		ble_uuid128_t base_uuid = BLE_SM_UUID_BASE;
		err_code = sd_ble_uuid_vs_add(&base_uuid, &p_ble_sm->uuid_type);
		if (err_code != NRF_SUCCESS)
		{
		return err_code;
		}
		
		ble_uuid.type = p_ble_sm->uuid_type;
		ble_uuid.uuid = BLE_SM_UUID_SERVICE;
		
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_ble_sm->service_handle);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

		// Add characteristics
		//FLAG SAH both read characteristics are not throwing errors
		err_code = sm_state_char_add(p_ble_sm, p_ble_sm_init);
		if (err_code != NRF_SUCCESS)
		{
			return err_code;
		}
		err_code = sm_shutter_char_add(p_ble_sm, p_ble_sm_init);
		if (err_code != NRF_SUCCESS)
		{
			return err_code;
		}
		err_code = sm_time_char_add(p_ble_sm, p_ble_sm_init);
		if (err_code != NRF_SUCCESS)
		{
			return err_code;
		}		
		err_code = sm_tl_pkt_char_add(p_ble_sm, p_ble_sm_init);
		if (err_code != NRF_SUCCESS)
		{
			return err_code;
		}
		
    // Add bleSM characteristic FLAG SAH this was called for in nAn-36
    return NRF_SUCCESS ;//ble_sm_char_add(p_ble_sm, p_ble_sm_init);
}


uint32_t ble_sm_state_update(ble_sm_t * p_ble_sm, uint8_t ble_state_info){
    uint32_t err_code = NRF_SUCCESS;

    if (ble_state_info != p_ble_sm->ble_sm_state_last)
    {	
	
		}
		return err_code;
}

