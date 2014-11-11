/** ble_state_machine.h

This was designed using the nAN-36_v1.1 application note from nordic

*/

#ifndef ble_SM_MACHINE_H_
#define ble_SM_MACHINE_H_

#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"


//#define BLE_SM_UUID_BASE {0xB6, 0x02, 0x72, 0x38, 0xCA, 0x82, 0x79, 0xC1, 0x10, 0x01,0x0C, 0x11, 0x26, 0xB3, 0x00, 0x00}
//as far As I can tell, the UUID base needs be inserted as an array in backwards order to what you want. The 0 values are what get
//witten over with the specific service UUIDs

#ifdef MICHRON
//UUID for michron is B602xxxx-7238-CA82-79C1-10010C1126B3
#define BLE_SM_UUID_BASE {0xB3, 0x26, 0x11, 0x0C, 0x01, 0x10, 0xC1, 0x79, 0x82, 0xCA, 0x38, 0x72,0x00, 0x00, 0x02, 0xB6 }

#else
//Radian UUID : C871xxxx-891F-8535-FC3C-D1BE7A6D3E65 
#define BLE_SM_UUID_BASE {0x65, 0x3E, 0x6D, 0x7A, 0xBE, 0xD1, 0x3C, 0xFC, 0x35, 0x85, 0x1F, 0x89,0x00, 0x00, 0x71, 0xC8 }

#endif

#define BLE_SM_UUID_SERVICE 0x1523
#define BLE_SM_UUID_STATE_CHAR 0x1524
#define BLE_SM_UUID_TIME_CHAR 0x1525
#define BLE_SM_UUID_SHUTTER_CHAR 0x1526
#define BLE_SM_UUID_TL_PKT_CHAR 0x1527



/**@brief ble State Machine Rate Service event type. */
typedef enum
{
    ble_SM_EVT_NOTIFICATION_ENABLED,                   /**< ble SM value notification enabled event. */
    ble_SM_EVT_NOTIFICATION_DISABLED                   /**< ble SM value notification disabled event. */
} ble_sm_evt_type_t;

/**@brief ble SM Service event. */
typedef struct
{
    ble_sm_evt_type_t evt_type;                        /**< Type of event. */
} ble_sm_evt_t;

// Forward declaration of the ble_sm_t type. 
typedef struct ble_sm_s ble_sm_t;

/**@brief ble State Machine Service event handler type. */
typedef void (*ble_sm_shutter_write_handler_t) (ble_sm_t * p_ble_sm, uint8_t*  vals_arr);

/**@brief ble State Machine Service event handler type. */
//we want to change this later on so that we can take in more than just 1 value
typedef void (*ble_sm_tl_pkt_write_handler_t) (ble_sm_t * p_ble_sm, uint8_t*  val);


/**@brief ble State Machine init structure. This contains all options and data needed for
 *        initialization of the service.*/
typedef struct
{
    ble_sm_shutter_write_handler_t  shutter_write_handler;    /**< Event handler to be called for handling shutter write events in the ble SM Service. */
    ble_sm_tl_pkt_write_handler_t   tl_pkt_write_handler;     /**< Event handler to be called for handling timelapse packet write events in the ble SM Service. */
		bool                          support_notification;           /**< TRUE if notification of ble State Machine information is supported. */
    ble_srv_report_ref_t *        p_report_ref;                   /**< If not NULL, a Report Reference descriptor with the specified value will be added to the Battery Level characteristic */
    uint8_t                       initial_ble_sm_state;             /**< Initial state machine state */
    ble_srv_cccd_security_mode_t  ble_sm_char_attr_md;     				/**< Initial security level for ble State Machine characteristics attribute */
    ble_gap_conn_sec_mode_t       ble_sm_report_read_perm; 				/**< Initial security level for ble state machine report read attribute */
		uint32_t											initial_ble_sm_time;								/**< initial state machine time passed to the ble state machine. */
} ble_sm_init_t;


/**@brief ble state machine Service structure. This contains various status information for the service. 
	This is used for referencing this instance of the service
*/
typedef struct ble_sm_s
{
	ble_sm_shutter_write_handler_t	shutter_write_handler;		/**< Event handler to be called for handling shutter write events in the ble SM Service. */
	ble_sm_tl_pkt_write_handler_t	 tl_pkt_write_handler;		 /**< Event handler to be called for handling timelapse packet write events in the ble SM Service. */
	uint16_t											service_handle;								 /**< Handle of ble SM Service (as provided by the BLE stack). */
	ble_gatts_char_handles_t			ble_sm_state_char_handles;						/**< Handles related to the ble SM state characteristic. */
	ble_gatts_char_handles_t			ble_sm_time_char_handles;						/**< Handles related to the ble SM time characteristic. */
	ble_gatts_char_handles_t			ble_sm_tl_pkt_char_handles;			/**< Handles related to the ble SM tl packet characteristic. */
	ble_gatts_char_handles_t			ble_sm_shutter_char_handles;					/**< Handles related to the ble SM shutter characteristic. */
	uint16_t											report_ref_handle;							/**< Handle of the Report Reference descriptor. */
	uint8_t											 ble_sm_state_last;						 /**< Last ble State passed to the ble state machine. */
	uint32_t											ble_sm_time_last;								/**< Last state machine time passed to the ble state machine. */
	uint16_t											conn_handle;										/**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
//		bool													is_notification_supported;			/**< TRUE if notification of ble State Machine is supported. */	
	uint8_t												uuid_type;
} ble_sm_t;

/**@brief Function for initializing the State Machine Service.
 *
 * @param[out]  p_ble_sm       state machine Service structure. This structure will have to be supplied by
 *                          the blelication. It will be initialized by this function, and will later
 *                          be used to identify this particular service instance.
 * @param[in]   p_ble_sm_init  Information needed to initialize the service.
 *
 * @return      NRF_SUCCESS on successful initialization of service, otherwise an error code.
 */
uint32_t ble_sm_init(ble_sm_t * p_ble_sm, const ble_sm_init_t * p_ble_sm_init);

/**@brief Function for handling the application's BLE Stack events.
 *
 * @details Handles all events from the BLE stack of interest to the Battery Service.
 *
 * @note For the requirements in the ble_sm specification to be fulfilled,
 *       ble state machine info has changed while the service has been disconnected from a bonded
 *       client.
 *
 * @param[in]   p_ble_sm      ble state machine Service structure.
 * @param[in]   p_ble_evt  		Event received from the BLE stack.
 */
void ble_sm_on_ble_evt(ble_sm_t * p_ble_sm, ble_evt_t * p_ble_evt);

/**@brief Function for updating the ble state machine info
 *
 * @details The blelication calls this function after having handled a SM event. If
 *          notification has been enabled, the State machine characteristic is sent to the client.
 *
 * @note For the requirements in the ble_SM specification to be fulfilled,
 *       this function must be called upon reconnection if the ble State Machine has changed
 *       while the service has been disconnected from a bonded client.
 *
 * @param[in]   p_ble_sm          ble state machine structure.
 * @param[in]   ble_state_info    New state machine information
 *
 * @return      NRF_SUCCESS on success, otherwise an error code.
 */
uint32_t ble_sm_state_update(ble_sm_t * p_ble_sm, uint8_t ble_state_info);


#endif //ble_SM_MACHINE_H_

/** @} */
