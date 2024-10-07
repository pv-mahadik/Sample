/**
 *   @file  data_path.c
 *
 *   @brief
 *      Implements Data path processing functionality.
 *
 *  \par
 *  NOTE:
 *      (C) Copyright 2018 Texas Instruments, Inc.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**************************************************************************
 *************************** Include Files ********************************
 **************************************************************************/

/* Standard Include Files. */
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include <xdc/runtime/System.h>

#include <ti/drivers/soc/soc.h>
#include <ti/drivers/hwa/hwa.h>
#include <ti/drivers/edma/edma.h>
#include <data_path.h>
#include <mmw.h>

/**
 *  @b Description
 *  @n
 *      Init HWA.
 */
void MmwDemo_hwaInit(MmwDemo_DataPathObj *obj)
{
    /* Initialize the HWA */
    HWA_init();
}

/**
 *  @b Description
 *  @n
 *      Init EDMA.
 */
void MmwDemo_edmaInit(MmwDemo_DataPathObj *obj)
{
    uint8_t edmaNumInstances, inst;
    int32_t errorCode;

    edmaNumInstances = EDMA_getNumInstances();
    for (inst = 0; inst < edmaNumInstances; inst++)
    {
        errorCode = EDMA_init(inst);
        if (errorCode != EDMA_NO_ERROR)
        {
            System_printf ("Debug: EDMA instance %d initialization returned error %d\n", errorCode);
            MmwDemo_debugAssert (0);
            return;
        }
        //System_printf ("Debug: EDMA instance %d has been initialized\n", inst);
    }

    memset(&obj->EDMA_errorInfo, 0, sizeof(obj->EDMA_errorInfo));
    memset(&obj->EDMA_transferControllerErrorInfo, 0, sizeof(obj->EDMA_transferControllerErrorInfo));
}

/**
 *  @b Description
 *  @n
 *      Call back function for EDMA CC (Channel controller) error as per EDMA API.
 *      Declare fatal error if happens, the output errorInfo can be examined if code
 *      gets trapped here.
 */
void MmwDemo_EDMA_errorCallbackFxn(EDMA_Handle handle, EDMA_errorInfo_t *errorInfo)
{
    gMmwMCB.dataPathObj.EDMA_errorInfo = *errorInfo;
    MmwDemo_debugAssert(0);
}

/**
 *  @b Description
 *  @n
 *      Call back function for EDMA transfer controller error as per EDMA API.
 *      Declare fatal error if happens, the output errorInfo can be examined if code
 *      gets trapped here.
 */
void MmwDemo_EDMA_transferControllerErrorCallbackFxn(EDMA_Handle handle,
                EDMA_transferControllerErrorInfo_t *errorInfo)
{
    gMmwMCB.dataPathObj.EDMA_transferControllerErrorInfo = *errorInfo;
    MmwDemo_debugAssert(0);
}

/**
 *  @b Description
 *  @n
 *      Checks for EDMA errors, used on devices where error interrupts are not connected
 *      to the CPU. Current use case is for LVDS, note it is not very useful to
 *      check for edma errors within the CBUFF session completion interrupts
 *      because they will not happen if edma had errors. So this API should be
 *      called at opportune places in the application code, typically at some time
 *      later than the triggering of the session when it is roughly expected that
 *      the session would have completed by that time.
 */
void MmwDemo_checkEdmaErrors(uint8_t edmaInstanceId)
{
    int32_t     errCode;
    bool        isAnyError;
    MmwDemo_DataPathObj *obj;
    uint8_t     idx = edmaInstanceId;

    obj = &gMmwMCB.dataPathObj;

    if (obj->isPollEdmaError[idx] == true)
    {
        errCode = EDMA_getErrorStatus(gMmwMCB.dataPathObj.edmaHandle[idx], &isAnyError, &obj->EDMA_errorInfo);
        if (errCode != EDMA_NO_ERROR)
        {
            System_printf("Error: EDMA_getErrorStatus() failed with error code = %d\n", errCode);
            MmwDemo_debugAssert(0);
        }

        if (isAnyError == true)
        {
            System_printf("EDMA channel controller has errors, see gMssMCB.EDMA_errorInfo\n");
            MmwDemo_debugAssert(0);
        }
    }

    if (obj->isPollEdmaTransferControllerError[idx] == true)
    {
        uint8_t tc;

        for(tc = 0; tc < obj->numEdmaEventQueues[idx]; tc++)
        {
            errCode = EDMA_getTransferControllerErrorStatus(obj->edmaHandle[idx], tc,
                           &isAnyError, &obj->EDMA_transferControllerErrorInfo);
            if (errCode != EDMA_NO_ERROR)
            {
                System_printf("Error: EDMA_getTransferControllerErrorStatus() failed with error code = %d\n", errCode);
                MmwDemo_debugAssert(0);
            }

            if (isAnyError == true)
            {
                System_printf("EDMA Transfer Controller instance %d has errors, see gMmwMCB.dataPathObj.EDMA_transferControllerErrorInfo\n", tc);
                MmwDemo_debugAssert(0);
            }
        }
    }
}

/**
 *  @b Description
 *  @n
 *      Open HWA instance.
 */
void MmwDemo_hwaOpen(MmwDemo_DataPathObj *obj, SOC_Handle socHandle)
{
    int32_t             errCode;

    /* Open the HWA Instance */
    obj->hwaHandle = HWA_open(0, socHandle, &errCode);
    if (obj->hwaHandle == NULL)
    {
        System_printf("Error: Unable to open the HWA Instance err:%d\n",errCode);
        MmwDemo_debugAssert (0);
        return;
    }
    //System_printf("Debug: HWA Instance %p has been opened successfully\n", obj->hwaHandle);
}

/**
 *  @b Description
 *  @n
 *      Close HWA instance.
 */
void MmwDemo_hwaClose(MmwDemo_DataPathObj *obj)
{
    int32_t             errCode;

    /* Close the HWA Instance */
    errCode = HWA_close(obj->hwaHandle);
    if (errCode != 0)
    {
        System_printf("Error: Unable to close the HWA Instance err:%d\n",errCode);
        MmwDemo_debugAssert (0);
        return;
    }
    //System_printf("Debug: HWA Instance %p has been closed successfully\n", obj->hwaHandle);
}

/**
 *  @b Description
 *  @n
 *      Open EDMA.
 */
void MmwDemo_edmaOpen(MmwDemo_DataPathObj *obj)
{
    int32_t             errCode;
    EDMA_instanceInfo_t edmaInstanceInfo;
    EDMA_errorConfig_t  errorConfig;
    int32_t             edmaCCIdx;
    uint8_t             tc;

    for (edmaCCIdx = 0; edmaCCIdx < EDMA_NUM_CC; edmaCCIdx++)
    {
        obj->edmaHandle[edmaCCIdx] = EDMA_open(edmaCCIdx, &errCode, &edmaInstanceInfo);
        obj->numEdmaEventQueues[edmaCCIdx] = edmaInstanceInfo.numEventQueues;

        if (obj->edmaHandle[edmaCCIdx] == NULL)
        {
            System_printf("Error: Unable to open the EDMA Instance err:%d\n",errCode);
            MmwDemo_debugAssert (0);
            return;
        }
        //System_printf("Debug: EDMA Instance %p has been opened successfully\n", obj->edmaHandle);

        errorConfig.isConfigAllEventQueues = true;
        errorConfig.isConfigAllTransferControllers = true;
        errorConfig.isEventQueueThresholdingEnabled = true;
        errorConfig.eventQueueThreshold = EDMA_EVENT_QUEUE_THRESHOLD_MAX;
        errorConfig.isEnableAllTransferControllerErrors = true;

        obj->isPollEdmaError[edmaCCIdx] = false;
        if (edmaInstanceInfo.isErrorInterruptConnected == true)
        {
            errorConfig.callbackFxn = MmwDemo_EDMA_errorCallbackFxn;
        }
        else
        {
            errorConfig.callbackFxn = NULL;
            obj->isPollEdmaError[edmaCCIdx] = true;
        }

        obj->isPollEdmaTransferControllerError[edmaCCIdx] = false;
        errorConfig.transferControllerCallbackFxn = MmwDemo_EDMA_transferControllerErrorCallbackFxn;

        for(tc = 0; tc < edmaInstanceInfo.numEventQueues; tc++)
        {
            if (edmaInstanceInfo.isTransferControllerErrorInterruptConnected[tc] == false)
            {
                errorConfig.transferControllerCallbackFxn = NULL;
                obj->isPollEdmaTransferControllerError[edmaCCIdx] = true;
                break;
            }
        }

        if ((errCode = EDMA_configErrorMonitoring(obj->edmaHandle[edmaCCIdx], &errorConfig)) != EDMA_NO_ERROR)
        {
            //System_printf("Error: EDMA_configErrorMonitoring() failed with errorCode = %d\n", errCode);
            MmwDemo_debugAssert (0);
            return;
        }
    }
}

/**
 *  @b Description
 *  @n
 *      Close EDMA.
 */
void MmwDemo_edmaClose(MmwDemo_DataPathObj *obj)
{
    EDMA_close(obj->edmaHandle);
}


/**
 *  @b Description
 *  @n
 *      Initializes data path object.
 */
void MmwDemo_dataPathObjInit(MmwDemo_DataPathObj *obj)
{
    memset(obj, 0, sizeof(MmwDemo_DataPathObj));
}
