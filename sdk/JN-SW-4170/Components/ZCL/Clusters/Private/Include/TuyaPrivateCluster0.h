#ifndef __TUYA_PRIVATE_CLUSTER0_H
#define __TUYA_PRIVATE_CLUSTER0_H

#include <jendefs.h>
#include "zcl.h"
#include "zcl_options.h"

/* Command codes */
typedef enum
{
    E_CLD_PRIVATE_0_CMD_00    = 0x00,
    E_CLD_PRIVATE_0_CMD_TUYA_RANDOM_TIME_CROND=0xF7,
    E_CLD_PRIVATE_0_CMD_TUYA_CYCLE_TIME_CROND=0xF8,
    E_CLD_PRIVATE_0_CMD_TUYA_INCHING_TIME_CROND=0xFB,
} teCLD_PrivateCluster0_Command;

typedef enum
{
    E_CLD_PRIVATE_ATTR_ID_RANDOM_TIME_CROND_TUYA    = 0xD001,
    E_CLD_PRIVATE_ATTR_ID_CYCLE_TIME_CROND_TUYA     = 0xD002,
    E_CLD_PRIVATE_ATTR_ID_SWITCH_TYPE_TUYA          = 0xD003,
} teCLD_Pivate_Cluster0_AttrID;

/* Definition of Basic Callback Event Structure */
typedef struct
{
    uint8          u8CommandId;
} tsCLD_Pivate_Cluster0_CallBackMessage_t;

typedef struct {
  uint8 onOff;
  uint16 duration;                   //Inching duration
  uint16 offElapse;
}tsCLD_TuyaInchingAction_t;

typedef struct {
  uint8 onOff;
  uint8 wday;      // 0x00 only execute once;   non-zero,week execute Bit6/SAT Bit5/FRI Bit4/Thurs etc.
  
  uint16 startTime;  // hour:startTime/60   minutes:  startTime%60
  uint16 stopTime;   // same as above
  uint16 onDuration; // 
  uint16 onElapse;
  uint16 offDuration;//
  uint16 offElapse;
}tsCLD_TuyaCondition_t;

typedef struct{
	uint8 crondVer;
	uint8 nodeLength;

	uint8 numTimeCrond;
	tsCLD_TuyaCondition_t cycleCrond[6];		 // cycle crond support maximum 6 total
}tsCLD_TuyaCycleTimeCrond_t;

typedef struct{
	uint8 crondVer;
	uint8 nodeLength;

	uint8 numTimeCrond;
	tsCLD_TuyaCondition_t randomCrond[10];		 // random crond support maximum 10 total
}tsCLD_TuyaRandomTimeCrond_t;

/* Private Cluster */
typedef struct
{
	tsCLD_TuyaInchingAction_t   sInchingAction;

    tsZCL_CharacterString       sTuyaSwitchType;           // inching mode or not
    zuint8                       au8TuyaSwitchType[4];

    tsCLD_TuyaCycleTimeCrond_t tctc;
    tsCLD_TuyaRandomTimeCrond_t trtc;

    tsZCL_Array       sTuyaCycleTimeCrond;
    tsZCL_Array       sTuyaRandomTimeCrond;
} tsCLD_PrivateCluster0_t;

/****************************************************************************/
/***        Macro Definitions                                             ***/
/****************************************************************************/

/* Cluster ID's */
// tuya private cluster
#define PRIVATE_CLUSTER_ID_TUYA_PRIVATE_ID0                      0xE000

/****************************************************************************/
/***        Exported Functions                                            ***/
/****************************************************************************/
PUBLIC  teZCL_Status eCLD_TuyaPrivate0CreateCustom(
    tsZCL_ClusterInstance                   *psClusterInstance,
    bool_t                                  bIsServer,
    tsZCL_ClusterDefinition                 *psClusterDefinition,
    void                                    *pvEndPointSharedStructPtr,
    uint8                                   *pu8AttributeControlBits);

/****************************************************************************/
/***        External Variables                                            ***/
/****************************************************************************/
extern tsZCL_ClusterDefinition sCLD_Private0Custom;
extern uint8 au8PrivateCluster0AttributeControlBits[];

#endif /* __TUYA_PRIVATE_CLUSTER0_H */
