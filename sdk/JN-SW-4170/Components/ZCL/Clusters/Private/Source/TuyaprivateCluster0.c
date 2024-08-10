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
#include "TuyaPrivateCluster0.h"
#include "dbg.h"
#include "base64.h"

/****************************************************************************/
/***        Exported Variables                                            ***/
/****************************************************************************/

/****************************************************************************/
/***        Local Variables                                               ***/
/****************************************************************************/

#ifdef ZCL_COMMAND_DISCOVERY_SUPPORTED
PRIVATE const tsZCL_CommandDefinition asCLD_PrivateClusterCommandDefinitions[] =
{
    {E_CLD_PRIVATE_0_CMD_00,              E_ZCL_CF_RX | E_ZCL_CF_TX}
};
#endif

PRIVATE const tsZCL_AttributeDefinition asCLD_PrivateClusterAttributeDefinitions[] =
{
	{E_CLD_PRIVATE_ATTR_ID_RANDOM_TIME_CROND_TUYA, (E_ZCL_AF_RP),  E_ZCL_ARRAY, (uint32)(&((tsCLD_PrivateCluster0_t*)(0))->sTuyaRandomTimeCrond), 0},
	{E_CLD_PRIVATE_ATTR_ID_CYCLE_TIME_CROND_TUYA,  (E_ZCL_AF_RP),  E_ZCL_ARRAY, (uint32)(&((tsCLD_PrivateCluster0_t*)(0))->sTuyaCycleTimeCrond), 0},
    {E_CLD_PRIVATE_ATTR_ID_SWITCH_TYPE_TUYA,       (E_ZCL_AF_RP),  E_ZCL_CSTRING, (uint32)(&((tsCLD_PrivateCluster0_t*)(0))->sTuyaSwitchType), 0},
};


tsZCL_ClusterDefinition sCLD_Private0Custom =
{
    PRIVATE_CLUSTER_ID_TUYA_PRIVATE_ID0,
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

uint8 au8PrivateCluster0AttributeControlBits[(sizeof(asCLD_PrivateClusterAttributeDefinitions) / sizeof(tsZCL_AttributeDefinition))];

// report attribute
PRIVATE teZCL_Status eCLD_TuyaPrivateReportInching(ZPS_tsAfEvent               *pZPSevent,
		tsCLD_PrivateCluster0_t *psCluster)
{
	teZCL_Status eStatus=E_ZCL_SUCCESS;
	uint8 nodeLength=4;    // base64 encode
	int out_len=0;
	uint8 src[3],dst[5];
	src[0]=psCluster->sInchingAction.onOff;
	src[1]=U16_UPPER_U8(psCluster->sInchingAction.duration);
	src[2]=U16_LOWER_U8(psCluster->sInchingAction.duration);
	base64_encode(src,sizeof(src),&out_len,dst);
	memcpy(psCluster->sTuyaSwitchType.pu8Data,dst,nodeLength);

    PDUM_thAPduInstance hAPduInst = hZCL_AllocateAPduInstance();

	tsZCL_Address sAddress;
    // build address structure
    eZCL_BuildTransmitAddressStructure(pZPSevent, &sAddress);

    // report tuya private attribute
    eStatus=eZCL_ReportAttribute(&sAddress,
    	PRIVATE_CLUSTER_ID_TUYA_PRIVATE_ID0,
    	E_CLD_PRIVATE_ATTR_ID_SWITCH_TYPE_TUYA,
    	pZPSevent->uEvent.sApsDataIndEvent.u8SrcEndpoint,
		pZPSevent->uEvent.sApsDataIndEvent.u8DstEndpoint,
    	hAPduInst);

	PDUM_eAPduFreeAPduInstance(hAPduInst);

	return eStatus;
}

// report attribute
PRIVATE teZCL_Status eCLD_TuyaPrivateReportCycleTimeCrond(ZPS_tsAfEvent               *pZPSevent,
		tsZCL_EndPointDefinition    *psEndPointDefinition,
		tsZCL_ClusterInstance       *psClusterInstance,
		tsZCL_HeaderParams          *psZCL_HeaderParams)
{
	teZCL_Status eStatus=E_ZCL_SUCCESS;
    tsZCL_CallBackEvent sZCL_CallBackEvent;
    tsCLD_Pivate_Cluster0_CallBackMessage_t sPrivateCallBackMessage;
	tsCLD_PrivateCluster0_t *psCluster=(tsCLD_PrivateCluster0_t*)psClusterInstance->pvEndPointSharedStructPtr;

	// crondVer 1 byte, nodeLength 1 byte, cycle crond maximum 60 byte,array type length 1 byte
	uint8 pData[2 + 60 + 1]={0};

	uint8 *p=pData;
	uint8 data_len=(2 + (psCluster->tctc.numTimeCrond * psCluster->tctc.nodeLength));

	// add version
	*p++ =	psCluster->tctc.crondVer;

	// add node length
	*p++ = psCluster->tctc.nodeLength;

	uint8 i=0;
	for(i=0;i<psCluster->tctc.numTimeCrond;i++)
	{
		*p++ = psCluster->tctc.cycleCrond[i].onOff;		
		*p++ = psCluster->tctc.cycleCrond[i].wday;

		*p++ = U16_UPPER_U8(psCluster->tctc.cycleCrond[i].startTime);
		*p++ = U16_LOWER_U8(psCluster->tctc.cycleCrond[i].startTime);

		*p++ = U16_UPPER_U8(psCluster->tctc.cycleCrond[i].stopTime);
		*p++ = U16_LOWER_U8(psCluster->tctc.cycleCrond[i].stopTime);

		*p++ = U16_UPPER_U8(psCluster->tctc.cycleCrond[i].onDuration);
		*p++ = U16_LOWER_U8(psCluster->tctc.cycleCrond[i].onDuration);

		*p++ = U16_UPPER_U8(psCluster->tctc.cycleCrond[i].offDuration);
		*p++ = U16_LOWER_U8(psCluster->tctc.cycleCrond[i].offDuration);
	}

	psCluster->sTuyaCycleTimeCrond.u16MaxLength=sizeof(pData);
	psCluster->sTuyaCycleTimeCrond.u16Length=data_len;
	psCluster->sTuyaCycleTimeCrond.pu8Data=pData;

    PDUM_thAPduInstance hAPduInst = hZCL_AllocateAPduInstance();

	tsZCL_Address sAddress;
    // build address structure
    eZCL_BuildTransmitAddressStructure(pZPSevent, &sAddress);

    // report tuya private attribute
    eStatus=eZCL_ReportAttribute(&sAddress,
    	PRIVATE_CLUSTER_ID_TUYA_PRIVATE_ID0,
    	E_CLD_PRIVATE_ATTR_ID_CYCLE_TIME_CROND_TUYA,
    	pZPSevent->uEvent.sApsDataIndEvent.u8SrcEndpoint,
		pZPSevent->uEvent.sApsDataIndEvent.u8DstEndpoint,
    	hAPduInst);

	PDUM_eAPduFreeAPduInstance(hAPduInst);

    sZCL_CallBackEvent.u8EndPoint           = psEndPointDefinition->u8EndPointNumber;
    sZCL_CallBackEvent.psClusterInstance    = psClusterInstance;
    sZCL_CallBackEvent.pZPSevent            = pZPSevent;
    sZCL_CallBackEvent.eEventType           = E_ZCL_CBET_CLUSTER_CUSTOM;
    sZCL_CallBackEvent.uMessage.sClusterCustomMessage.u16ClusterId = PRIVATE_CLUSTER_ID_TUYA_PRIVATE_ID0;
    sZCL_CallBackEvent.uMessage.sClusterCustomMessage.pvCustomData = (void*)&sPrivateCallBackMessage;

    /* Fill in message */
    sPrivateCallBackMessage.u8CommandId = psZCL_HeaderParams->u8CommandIdentifier;

    // call callback
    psEndPointDefinition->pCallBackFunctions(&sZCL_CallBackEvent);		

	return eStatus;
}

// report attribute
PRIVATE teZCL_Status eCLD_TuyaPrivateReportRandomTimeCrond(ZPS_tsAfEvent               *pZPSevent,
                            tsZCL_EndPointDefinition    *psEndPointDefinition,
                            tsZCL_ClusterInstance       *psClusterInstance,
                            tsZCL_HeaderParams          *psZCL_HeaderParams)
{
	teZCL_Status eStatus=E_ZCL_SUCCESS;
    tsZCL_CallBackEvent sZCL_CallBackEvent;
    tsCLD_Pivate_Cluster0_CallBackMessage_t sPrivateCallBackMessage;
	tsCLD_PrivateCluster0_t *psCluster=(tsCLD_PrivateCluster0_t*)psClusterInstance->pvEndPointSharedStructPtr;

	// crondVer 1 byte, nodeLength 1 byte, random crond maximum 10*6 byte,array type length 1 byte
	uint8 pData[2 + 60 + 1]={0};

	uint8 *p=pData;
	uint8 data_len=(2 + (psCluster->trtc.numTimeCrond * psCluster->trtc.nodeLength));

	// add version
	*p++ =	psCluster->trtc.crondVer;

	// add node length
	*p++ = psCluster->trtc.nodeLength;

	uint8 i=0;
	for(i=0;i<psCluster->trtc.numTimeCrond;i++)
	{
		*p++ = psCluster->trtc.randomCrond[i].onOff;		
		*p++ = psCluster->trtc.randomCrond[i].wday;

		*p++ = U16_UPPER_U8(psCluster->trtc.randomCrond[i].startTime);
		*p++ = U16_LOWER_U8(psCluster->trtc.randomCrond[i].startTime);

		*p++ = U16_UPPER_U8(psCluster->trtc.randomCrond[i].stopTime);
		*p++ = U16_LOWER_U8(psCluster->trtc.randomCrond[i].stopTime);
	}

	psCluster->sTuyaRandomTimeCrond.u16MaxLength=sizeof(pData);
	psCluster->sTuyaRandomTimeCrond.u16Length=data_len;
	psCluster->sTuyaRandomTimeCrond.pu8Data=pData;

    PDUM_thAPduInstance hAPduInst = hZCL_AllocateAPduInstance();

	tsZCL_Address sAddress;
    // build address structure
    eZCL_BuildTransmitAddressStructure(pZPSevent, &sAddress);

    // report tuya private attribute
    eStatus=eZCL_ReportAttribute(&sAddress,
    	PRIVATE_CLUSTER_ID_TUYA_PRIVATE_ID0,
    	E_CLD_PRIVATE_ATTR_ID_RANDOM_TIME_CROND_TUYA,
    	pZPSevent->uEvent.sApsDataIndEvent.u8SrcEndpoint,
		pZPSevent->uEvent.sApsDataIndEvent.u8DstEndpoint,
    	hAPduInst);

	PDUM_eAPduFreeAPduInstance(hAPduInst);

    sZCL_CallBackEvent.u8EndPoint           = psEndPointDefinition->u8EndPointNumber;
    sZCL_CallBackEvent.psClusterInstance    = psClusterInstance;
    sZCL_CallBackEvent.pZPSevent            = pZPSevent;
    sZCL_CallBackEvent.eEventType           = E_ZCL_CBET_CLUSTER_CUSTOM;
    sZCL_CallBackEvent.uMessage.sClusterCustomMessage.u16ClusterId = PRIVATE_CLUSTER_ID_TUYA_PRIVATE_ID0;
    sZCL_CallBackEvent.uMessage.sClusterCustomMessage.pvCustomData = (void*)&sPrivateCallBackMessage;

    /* Fill in message */
    sPrivateCallBackMessage.u8CommandId = psZCL_HeaderParams->u8CommandIdentifier;

    // call callback
    psEndPointDefinition->pCallBackFunctions(&sZCL_CallBackEvent);

	return eStatus;
}

PRIVATE  teZCL_Status eCLD_TuyaPrivateInchingHandle(
                            ZPS_tsAfEvent               *pZPSevent,
                            tsZCL_EndPointDefinition    *psEndPointDefinition,
                            tsZCL_ClusterInstance       *psClusterInstance)
{
	teZCL_Status eStatus=E_ZCL_SUCCESS;
    uint16 u16Offset;
    int out_len=0;
    tsZCL_HeaderParams sZCL_HeaderParams;
    tsZCL_CallBackEvent sZCL_CallBackEvent;
    tsCLD_Pivate_Cluster0_CallBackMessage_t sPrivateCallBackMessage;
    uint8 nodeLength=4;    // base64 encode

    tsCLD_PrivateCluster0_t *psCluster=(tsCLD_PrivateCluster0_t*)psClusterInstance->pvEndPointSharedStructPtr;

    // parse command header
    u16Offset = u16ZCL_ReadCommandHeader(pZPSevent->uEvent.sApsDataIndEvent.hAPduInst,
                             &sZCL_HeaderParams);
                             
    //sZCL_HeaderParams.u8TransactionSequenceNumber;

	// get start
	uint8 *pu8Start = (uint8 *)(PDUM_pvAPduInstanceGetPayload(pZPSevent->uEvent.sApsDataIndEvent.hAPduInst)) + u16Offset;
	uint8 out[3]={0};
	uint8 result=base64_decode((unsigned char *)pu8Start, nodeLength, &out_len, out);
	if(result == 0)
	{
		return (E_ZCL_FAIL);
	}

	psCluster->sInchingAction.onOff= U16_LOWER_U8(out[0]);
	psCluster->sInchingAction.duration=BYTE_ORDER_16(out[2],out[1]);

	psCluster->sTuyaSwitchType.u8Length=(nodeLength);

    sZCL_CallBackEvent.u8EndPoint           = psEndPointDefinition->u8EndPointNumber;
    sZCL_CallBackEvent.psClusterInstance    = psClusterInstance;
    sZCL_CallBackEvent.pZPSevent            = pZPSevent;
    sZCL_CallBackEvent.eEventType           = E_ZCL_CBET_CLUSTER_CUSTOM;
    sZCL_CallBackEvent.uMessage.sClusterCustomMessage.u16ClusterId = PRIVATE_CLUSTER_ID_TUYA_PRIVATE_ID0;
    sZCL_CallBackEvent.uMessage.sClusterCustomMessage.pvCustomData = (void*)&sPrivateCallBackMessage;

    /* Fill in message */
    sPrivateCallBackMessage.u8CommandId = sZCL_HeaderParams.u8CommandIdentifier;

    // call callback
    psEndPointDefinition->pCallBackFunctions(&sZCL_CallBackEvent);	

	eStatus=eCLD_TuyaPrivateReportInching(pZPSevent,psCluster);

    return(eStatus);
}

PRIVATE  teZCL_Status eCLD_TuyaPrivateRandomCrondHandle(
                            ZPS_tsAfEvent               *pZPSevent,
                            tsZCL_EndPointDefinition    *psEndPointDefinition,
                            tsZCL_ClusterInstance       *psClusterInstance)
{
	teZCL_Status eStatus=E_ZCL_SUCCESS;
    uint16 u16Offset;
    tsZCL_HeaderParams sZCL_HeaderParams;

    tsCLD_PrivateCluster0_t *psCluster=(tsCLD_PrivateCluster0_t*)psClusterInstance->pvEndPointSharedStructPtr;

    // parse command header
    u16Offset = u16ZCL_ReadCommandHeader(pZPSevent->uEvent.sApsDataIndEvent.hAPduInst,
                             &sZCL_HeaderParams);

	// get start
	uint8 *pu8Start = (uint8 *)(PDUM_pvAPduInstanceGetPayload(pZPSevent->uEvent.sApsDataIndEvent.hAPduInst)) + u16Offset;
	uint16 u16payloadSize = PDUM_u16APduInstanceGetPayloadSize(pZPSevent->uEvent.sApsDataIndEvent.hAPduInst);
	DBG_vPrintf(1, "payload size:%d %x %x %x\n",u16payloadSize,pu8Start[0],pu8Start[1],pu8Start[2]);

	if(u16payloadSize < 2)
	{
		eStatus=E_ZCL_ERR_ATTRIBUTE_RANGE;
	}else{
		u16payloadSize -= 3;     // skip 3 byte: frame control,TSN,cluster command

		uint8 crondVer = (uint8)pu8Start[0];
		uint8 nodeLength=(uint8)pu8Start[1];
		pu8Start += 2;
		
		uint8 numTimeCrond=u16payloadSize/nodeLength;

		if((nodeLength == 0x06) && (numTimeCrond <= 10)) // maximum support 10
		{
			uint8 i=0;
			psCluster->trtc.numTimeCrond=numTimeCrond;
			psCluster->trtc.crondVer=crondVer;
			psCluster->trtc.nodeLength=nodeLength;
			for(i=0;i<numTimeCrond;i++)
			{
				psCluster->trtc.randomCrond[i].onOff=U16_LOWER_U8(pu8Start[0]);
				psCluster->trtc.randomCrond[i].wday=U16_LOWER_U8(pu8Start[1]);
				psCluster->trtc.randomCrond[i].startTime=BYTE_ORDER_16(pu8Start[3],pu8Start[2]);
				psCluster->trtc.randomCrond[i].stopTime=BYTE_ORDER_16(pu8Start[5],pu8Start[4]);

				psCluster->trtc.randomCrond[i].onDuration=0;
				psCluster->trtc.randomCrond[i].offDuration=0;

				psCluster->trtc.randomCrond[i].onElapse=0;
				psCluster->trtc.randomCrond[i].offElapse=0;
				
				pu8Start += nodeLength;
			}

			eStatus=eCLD_TuyaPrivateReportRandomTimeCrond(pZPSevent,
				psEndPointDefinition,psClusterInstance,&sZCL_HeaderParams);
		}else{
			eStatus=E_ZCL_ERR_ATTRIBUTE_MISMATCH;
		}
	}

	return(eStatus);
}

PRIVATE  teZCL_Status eCLD_TuyaPrivateCycleCrondHandle(
                            ZPS_tsAfEvent               *pZPSevent,
                            tsZCL_EndPointDefinition    *psEndPointDefinition,
                            tsZCL_ClusterInstance       *psClusterInstance)
{
	teZCL_Status eStatus=E_ZCL_SUCCESS;
    uint16 u16Offset;
    tsZCL_HeaderParams sZCL_HeaderParams;

    tsCLD_PrivateCluster0_t *psCluster=(tsCLD_PrivateCluster0_t*)psClusterInstance->pvEndPointSharedStructPtr;

    // parse command header
    u16Offset = u16ZCL_ReadCommandHeader(pZPSevent->uEvent.sApsDataIndEvent.hAPduInst,
                             &sZCL_HeaderParams);

	// get start
	uint8 *pu8Start = (uint8 *)(PDUM_pvAPduInstanceGetPayload(pZPSevent->uEvent.sApsDataIndEvent.hAPduInst)) + u16Offset;
	uint16 u16payloadSize = PDUM_u16APduInstanceGetPayloadSize(pZPSevent->uEvent.sApsDataIndEvent.hAPduInst);

	if(u16payloadSize < 2)
	{
		eStatus=E_ZCL_ERR_ATTRIBUTE_RANGE;
	}else{
		u16payloadSize -= 3;     // skip 3 byte: frame control,TSN,cluster command

		uint8 crondVer = (uint8)pu8Start[0];
		uint8 nodeLength=(uint8)pu8Start[1];
		pu8Start += 2;
		
		uint8 numTimeCrond=u16payloadSize/nodeLength;

		if((nodeLength == 0x0A) && (numTimeCrond <= 6)) // maximum support 6
		{
			uint8 i=0;
			psCluster->tctc.numTimeCrond=numTimeCrond;
			psCluster->tctc.crondVer=crondVer;
			psCluster->tctc.nodeLength=nodeLength;
			for(i=0;i<numTimeCrond;i++)
			{
				psCluster->tctc.cycleCrond[i].onOff=U16_LOWER_U8(pu8Start[0]);
				psCluster->tctc.cycleCrond[i].wday=U16_LOWER_U8(pu8Start[1]);
				psCluster->tctc.cycleCrond[i].startTime=BYTE_ORDER_16(pu8Start[3],pu8Start[2]);
				psCluster->tctc.cycleCrond[i].stopTime=BYTE_ORDER_16(pu8Start[5],pu8Start[4]);
				psCluster->tctc.cycleCrond[i].onDuration=BYTE_ORDER_16(pu8Start[7],pu8Start[6]);
				psCluster->tctc.cycleCrond[i].offDuration=BYTE_ORDER_16(pu8Start[9],pu8Start[8]);

				psCluster->tctc.cycleCrond[i].onElapse=0;
				psCluster->tctc.cycleCrond[i].offElapse=0;

				pu8Start += nodeLength;
			}

			eStatus=eCLD_TuyaPrivateReportCycleTimeCrond(pZPSevent,
				psEndPointDefinition,psClusterInstance,&sZCL_HeaderParams);
		}else{
			eStatus=E_ZCL_ERR_ATTRIBUTE_MISMATCH;
		}
	}

	return(eStatus);
}

PRIVATE teZCL_Status eCLD_PrivateCommandHandler(
    ZPS_tsAfEvent               *pZPSevent,
    tsZCL_EndPointDefinition    *psEndPointDefinition,
    tsZCL_ClusterInstance       *psClusterInstance)
{
    teZCL_Status eReturnStatus = E_ZCL_SUCCESS;

	tsZCL_HeaderParams sZCL_HeaderParams;
	// further error checking can only be done once we have interrogated the ZCL payload
	u16ZCL_ReadCommandHeader(pZPSevent->uEvent.sApsDataIndEvent.hAPduInst,
							 &sZCL_HeaderParams);

	// We don't handle messages as a client, so exit
	if(psClusterInstance->bIsServer == FALSE)
	{
		return(E_ZCL_FAIL);
	}

	/* 
	 * The command is successfully qualified for all the errors , now take action 
	 */
	// get EP mutex
#ifndef COOPERATIVE
		eZCL_GetMutex(psEndPointDefinition);
#endif

	// SERVER
	switch(sZCL_HeaderParams.u8CommandIdentifier)
	{
		case E_CLD_PRIVATE_0_CMD_TUYA_RANDOM_TIME_CROND:
		{
			eReturnStatus=eCLD_TuyaPrivateRandomCrondHandle(pZPSevent, psEndPointDefinition, psClusterInstance);
		}
		break;

		case E_CLD_PRIVATE_0_CMD_TUYA_CYCLE_TIME_CROND:
		{
			eReturnStatus=eCLD_TuyaPrivateCycleCrondHandle(pZPSevent, psEndPointDefinition, psClusterInstance);
		}
		break;

		case E_CLD_PRIVATE_0_CMD_TUYA_INCHING_TIME_CROND:
		{
			eReturnStatus=eCLD_TuyaPrivateInchingHandle(pZPSevent, psEndPointDefinition, psClusterInstance);
			break;
		}
		
		default:
		{
			eReturnStatus = E_ZCL_ERR_CUSTOM_COMMAND_HANDLER_NULL_OR_RETURNED_ERROR;
			break;
		}
	}

	// unlock and return
#ifndef COOPERATIVE
		eZCL_ReleaseMutex(psEndPointDefinition);
#endif

    // delete the i/p buffer on return
    return(eReturnStatus);
}

PUBLIC  teZCL_Status eCLD_TuyaPrivate0CreateCustom(
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
    		tsCLD_PrivateCluster0_t *psCluster=(tsCLD_PrivateCluster0_t*)pvEndPointSharedStructPtr;

    		psCluster->sTuyaSwitchType.u8MaxLength=sizeof(psCluster->au8TuyaSwitchType);
    		psCluster->sTuyaSwitchType.u8Length=0;
    		psCluster->sTuyaSwitchType.pu8Data=psCluster->au8TuyaSwitchType;

			psCluster->sTuyaCycleTimeCrond.u16MaxLength=0;
			psCluster->sTuyaCycleTimeCrond.u16Length=0;
			psCluster->sTuyaCycleTimeCrond.pu8Data=NULL;

			psCluster->sTuyaRandomTimeCrond.u16MaxLength=0;
			psCluster->sTuyaRandomTimeCrond.u16Length=0;
			psCluster->sTuyaRandomTimeCrond.pu8Data=NULL;			
        }
    }

    /* As this cluster has reportable attributes enable default reporting */
    vZCL_SetDefaultReporting(psClusterInstance);

    return E_ZCL_SUCCESS;
}
