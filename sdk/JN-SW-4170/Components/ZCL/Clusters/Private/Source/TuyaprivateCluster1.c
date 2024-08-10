/****************************************************************************/
/***        Include files                                                 ***/
/****************************************************************************/
#include <jendefs.h>

#include "zps_apl.h"
#include "zps_apl_aib.h"

#include "zcl.h"
#include "zcl_customcommand.h"
#include "zcl_options.h"
#include "string.h"
#include "TuyaPrivateCluster1.h"

#include "dbg.h"

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

#ifdef ZCL_COMMAND_DISCOVERY_SUPPORTED
PRIVATE const tsZCL_CommandDefinition asCLD_PrivateClusterCommandDefinitions[] =
{
    {E_CLD_PRIVATE_1_CMD_00,              E_ZCL_CF_RX | E_ZCL_CF_TX}
};
#endif

PRIVATE const tsZCL_AttributeDefinition asCLD_PrivateClusterAttributeDefinitions[] =
{
    {E_CLD_PRIVATE_ATTR_ID_SWITCH_MODE_TUYA,       (E_ZCL_AF_RD|E_ZCL_AF_WR|E_ZCL_AF_RP),  E_ZCL_ENUM8, (uint32)(&((tsCLD_PrivateCluster1_t*)(0))->sTuyaSwitchMode), 0},
};

tsZCL_ClusterDefinition sCLD_Private1Custom =
{
    PRIVATE_CLUSTER_ID_TUYA_PRIVATE_ID1,
    FALSE,
    E_ZCL_SECURITY_NETWORK,
    (sizeof(asCLD_PrivateClusterAttributeDefinitions) / sizeof(tsZCL_AttributeDefinition)),
    (tsZCL_AttributeDefinition*)asCLD_PrivateClusterAttributeDefinitions,
    NULL
#ifdef ZCL_COMMAND_DISCOVERY_SUPPORTED
    ,
    (sizeof(asCLD_PrivateClusterCommandDefinitions) / sizeof(tsZCL_CommandDefinition)),
    (tsZCL_CommandDefinition*)asCLD_PrivateClusterCommandDefinitions
#endif
};

uint8 au8PrivateCluster1AttributeControlBits[(sizeof(asCLD_PrivateClusterAttributeDefinitions) / sizeof(tsZCL_AttributeDefinition))];

PRIVATE teZCL_Status eCLD_PrivateCommandHandler(
    ZPS_tsAfEvent               *pZPSevent,
    tsZCL_EndPointDefinition    *psEndPointDefinition,
    tsZCL_ClusterInstance       *psClusterInstance)
{
    teZCL_Status eReturnStatus = E_ZCL_SUCCESS;

    // delete the i/p buffer on return
    return(eReturnStatus);
}

PUBLIC  teZCL_Status eCLD_TuyaPrivate1CreateCustom(
    tsZCL_ClusterInstance                   *psClusterInstance,
    bool_t                                  bIsServer,
    tsZCL_ClusterDefinition                 *psClusterDefinition,
    void                                    *pvEndPointSharedStructPtr,
    uint8                                   *pu8AttributeControlBits)
{
#ifdef STRICT_PARAM_CHECK
    /* Parameter check */
    if((psClusterInstance == NULL) ||
       (psClusterDefinition == NULL))
    {
        return E_ZCL_ERR_PARAMETER_NULL;
    }
#endif

    // cluster data
    vZCL_InitializeClusterInstance(
        psClusterInstance,
        bIsServer,
        psClusterDefinition,
        pvEndPointSharedStructPtr,
        pu8AttributeControlBits,
        NULL,
        eCLD_PrivateCommandHandler);

    if(psClusterInstance->pvEndPointSharedStructPtr != NULL)
    {
    	if(bIsServer)
    	{
        	((tsCLD_PrivateCluster1_t*)pvEndPointSharedStructPtr)->sTuyaSwitchMode=0;	
        }
    }

    /* As this cluster has reportable attributes enable default reporting */
    vZCL_SetDefaultReporting(psClusterInstance);

    return E_ZCL_SUCCESS;
}
