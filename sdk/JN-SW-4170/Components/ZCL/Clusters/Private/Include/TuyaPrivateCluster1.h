#ifndef __TUYA_PRIVATE_CLUSTER1_H
#define __TUYA_PRIVATE_CLUSTER1_H

#include <jendefs.h>
#include "zcl.h"
#include "zcl_options.h"

/* Command codes */
typedef enum
{
    E_CLD_PRIVATE_1_CMD_00    = 0x00,
} teCLD_PrivateCluster1_Command;

typedef enum
{
    E_CLD_PRIVATE_ATTR_ID_SWITCH_MODE_TUYA          = 0xD030,
} teCLD_Pivate_Cluster1_AttrID;

/* Private Cluster */
typedef struct
{
	zenum8                      sTuyaSwitchMode;           // 0x00: toggle, 0x01: state, 0x02: momentary
} tsCLD_PrivateCluster1_t;

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/* Cluster ID's */
// tuya private cluster
#define PRIVATE_CLUSTER_ID_TUYA_PRIVATE_ID1                      0xE001

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
PUBLIC  teZCL_Status eCLD_TuyaPrivate1CreateCustom(
    tsZCL_ClusterInstance                   *psClusterInstance,
    bool_t                                  bIsServer,
    tsZCL_ClusterDefinition                 *psClusterDefinition,
    void                                    *pvEndPointSharedStructPtr,
    uint8                                   *pu8AttributeControlBits);

/****************************************************************************/
/***        External Variables                                            ***/
/****************************************************************************/
extern tsZCL_ClusterDefinition sCLD_Private1Custom;
extern uint8 au8PrivateCluster1AttributeControlBits[];

#endif /* __TUYA_PRIVATE_CLUSTER1_H */
