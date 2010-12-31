/*
 * templates.c
 *
 * Copyright(c) 1998 - 2009 Texas Instruments. All rights reserved.      
 * All rights reserved.                                                  
 *                                                                       
 * Redistribution and use in source and binary forms, with or without    
 * modification, are permitted provided that the following conditions    
 * are met:                                                              
 *                                                                       
 *  * Redistributions of source code must retain the above copyright     
 *    notice, this list of conditions and the following disclaimer.      
 *  * Redistributions in binary form must reproduce the above copyright  
 *    notice, this list of conditions and the following disclaimer in    
 *    the documentation and/or other materials provided with the         
 *    distribution.                                                      
 *  * Neither the name Texas Instruments nor the names of its            
 *    contributors may be used to endorse or promote products derived    
 *    from this software without specific prior written permission.      
 *                                                                       
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS   
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT     
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT      
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT   
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file reportReplvl.c
 *  \brief Report level implementation
 *
 *  \see reportReplvl.h
 */

/***************************************************************************/
/*																		   */
/*		MODULE:	reportReplvl.c											   */
/*    PURPOSE:	Report level implementation	 							   */
/*																		   */
/***************************************************************************/

#define __FILE_ID__  FILE_ID_88
#include "tidef.h"
#include "report.h"
#include "osApi.h"
#include "siteHash.h"
#include "rate.h"
#include "rsnApi.h"
#include "regulatoryDomainApi.h"
#include "siteMgrApi.h"
#include "TWDriver.h"
#include "StaCap.h"

/********************************************/
/*		Functions Implementations			*/
/********************************************/

/************************************************************************
 *                        buildNullTemplate								*
 ************************************************************************
DESCRIPTION: This function build a NULL data template to set to the HAL 
				when joining an infrastructure network
				performs the following:
				-	Build a template & set the template len, the template type is set in the site mgr
                                                                                                   
INPUT:      pSiteMgr	-	Handle to site manager	
			pTemplate	-	Pointer to the template structure		


OUTPUT:		


RETURN:     TI_OK

************************************************************************/
TI_STATUS buildNullTemplate(siteMgr_t *pSiteMgr, TSetTemplate *pTemplate)
{
	paramInfo_t			param;
	TI_UINT32				size;
	nullDataTemplate_t	*pBuffer = (nullDataTemplate_t	*)pTemplate->ptr;
	siteEntry_t			*pPrimarySite = pSiteMgr->pSitesMgmtParams->pPrimarySite;
	TI_UINT16				fc;

	os_memoryZero(pSiteMgr->hOs, pBuffer, sizeof(nullDataTemplate_t));

	/*
	 * Header First
	 */
	/* Set destination address */
	MAC_COPY (pBuffer->hdr.DA, pPrimarySite->bssid);  

	/* Set BSSID address */
	MAC_COPY (pBuffer->hdr.BSSID, pPrimarySite->bssid);  

	/* Build Source address */
	param.paramType = CTRL_DATA_MAC_ADDRESS;
	ctrlData_getParam(pSiteMgr->hCtrlData, &param);
	MAC_COPY (pBuffer->hdr.SA, param.content.ctrlDataDeviceMacAddress);  
	
	fc = DOT11_FC_DATA_NULL_FUNCTION;
	fc |= (TI_TRUE << DOT11_FC_TO_DS_SHIFT);

	COPY_WLAN_WORD(&pBuffer->hdr.fc, &fc); /* copy with endianess handling. */

	size = sizeof(dot11_mgmtHeader_t);

	pTemplate->len = size;
	
	return TI_OK;
}

/************************************************************************
 *                        buildDisconnTemplate								*
 ************************************************************************
DESCRIPTION: This function build a Death/Disassoc template to set to the HAL 
				when joining an infrastructure network
				performs the following:
				-	Build a template & set the template len, the template type is set in the site mgr
                                                                                                   
INPUT:      pSiteMgr	-	Handle to site manager	
			pTemplate	-	Pointer to the template structure		


OUTPUT:		


RETURN:     TI_OK

************************************************************************/
TI_STATUS buildDisconnTemplate(siteMgr_t *pSiteMgr, TSetTemplate *pTemplate)
{
	paramInfo_t			param;
	TI_UINT32				size;
	disconnTemplate_t	*pBuffer = (disconnTemplate_t	*)pTemplate->ptr;
	siteEntry_t			*pPrimarySite = pSiteMgr->pSitesMgmtParams->pPrimarySite;
	TI_UINT16				fc;

	os_memoryZero(pSiteMgr->hOs, pBuffer, sizeof(disconnTemplate_t));

	/*
	 * Header First
	 */
	/* Set destination address */
	MAC_COPY (pBuffer->hdr.DA, pPrimarySite->bssid);  

	/* Set BSSID address */
	MAC_COPY (pBuffer->hdr.BSSID, pPrimarySite->bssid);  

	/* Build Source address */
	param.paramType = CTRL_DATA_MAC_ADDRESS;
	ctrlData_getParam(pSiteMgr->hCtrlData, &param);
	MAC_COPY (pBuffer->hdr.SA, param.content.ctrlDataDeviceMacAddress);  
	
	fc = DOT11_FC_DISASSOC; /* will be change by firmware to DOT11_FC_DEAUTH if needed */
	fc |= (TI_TRUE << DOT11_FC_TO_DS_SHIFT);

	COPY_WLAN_WORD(&pBuffer->hdr.fc, &fc); /* copy with endianess handling. */

	pBuffer->disconnReason = 0; /* filled by firmware */
	
	size = sizeof(disconnTemplate_t);

	pTemplate->len = size;
	
	return TI_OK;
}

/** 
 * \fn     setDefaultProbeReqTemplate 
 * \brief  set Default Probe Req Template tp the FW.
 * 
 * set Default Probe Req Template tp the FW.
 * 
 * \param  hSiteMgr	-	Handle to site manager 
 * \return None
 * \sa     
 */ 
void setDefaultProbeReqTemplate (TI_HANDLE	hSiteMgr)
{
    siteMgr_t	*pSiteMgr = (siteMgr_t *)hSiteMgr;
    TSetTemplate        tTemplateStruct;
    probeReqTemplate_t  tProbeReqTemplate;
    TSsid               tBroadcastSSID;

    /* 
     * Setting probe request temapltes for both bands.
     * allocating EMPTY 32 bytes for the SSID IE, to reserve space for different SSIDs the FW will set
     */
    tBroadcastSSID.len = MAX_SSID_LEN;
    os_memorySet (pSiteMgr->hOs, &(tBroadcastSSID.str[ 0 ]), 0, MAX_SSID_LEN);
    tTemplateStruct.ptr = (TI_UINT8 *)&tProbeReqTemplate;
    tTemplateStruct.type = PROBE_REQUEST_TEMPLATE;
    tTemplateStruct.eBand = RADIO_BAND_2_4_GHZ;
    tTemplateStruct.uRateMask = RATE_MASK_UNSPECIFIED;
    buildProbeReqTemplate (hSiteMgr, &tTemplateStruct, &tBroadcastSSID, RADIO_BAND_2_4_GHZ);
    TWD_CmdTemplate (pSiteMgr->hTWD, &tTemplateStruct, NULL, NULL);
    tTemplateStruct.eBand = RADIO_BAND_5_0_GHZ;
    buildProbeReqTemplate (hSiteMgr, &tTemplateStruct, &tBroadcastSSID, RADIO_BAND_5_0_GHZ);
    TWD_CmdTemplate (pSiteMgr->hTWD, &tTemplateStruct, NULL, NULL);
}

/************************************************************************
 *                        buildProbeReqTemplate							*
 ************************************************************************
DESCRIPTION: This function build a probe request template to set to the HAL in the scan process.
				performs the following:
				-	Build a template & set the template len, the template type is set in the site mgr
                                                                                                   
INPUT:      pSiteMgr	-	Handle to site manager	
			pTemplate	-	Pointer to the template structure		
			pSsid		-	Desired SSID


OUTPUT:		


RETURN:     TI_OK

************************************************************************/
TI_STATUS buildProbeReqTemplate(siteMgr_t *pSiteMgr, TSetTemplate *pTemplate, TSsid *pSsid, ERadioBand radioBand)
{
	paramInfo_t			param;
	char				*pBuf;
	int i;
	probeReqTemplate_t	*pBuffer = (probeReqTemplate_t	*)pTemplate->ptr;
	TI_UINT32			 size;
	dot11_RATES_t		*pDot11Rates;	
	TI_UINT32			 len = 0, ofdmIndex = 0;
	TI_UINT32			 suppRatesLen, extSuppRatesLen;
	TI_UINT8			 ratesBuf[DOT11_MAX_SUPPORTED_RATES];
	TI_UINT8             WSCOuiIe[4] = { 0x00, 0x50, 0xf2, 0x04};
	TI_UINT32			 supportedRateMask,basicRateMask;	
	TI_UINT16			 fc = DOT11_FC_PROBE_REQ;

	os_memoryZero(pSiteMgr->hOs, pBuffer, sizeof(probeReqTemplate_t));

	/*
	 * Header First
	 */
	/* Set destination address */
	for (i = 0; i < MAC_ADDR_LEN; i++)
		pBuffer->hdr.DA[i] = 0xFF;

	/* Set BSSID address */

	for (i = 0; i < MAC_ADDR_LEN; i++)
		pBuffer->hdr.BSSID[i] = 0xFF;
 

	/* Build Source address */
	param.paramType = CTRL_DATA_MAC_ADDRESS;
	ctrlData_getParam(pSiteMgr->hCtrlData, &param);
	MAC_COPY (pBuffer->hdr.SA, param.content.ctrlDataDeviceMacAddress);  
	
	COPY_WLAN_WORD(&pBuffer->hdr.fc, &fc); /* copy with endianess handling. */

	size = sizeof(dot11_mgmtHeader_t);
	pBuf = (char *)&(pBuffer->infoElements);
	
   /*
	* Informataion elements
	*/
	/* SSID */
	((dot11_SSID_t *)(pBuf))->hdr[0] = DOT11_SSID_ELE_ID;
	((dot11_SSID_t *)(pBuf))->hdr[1] = pSsid->len;
	os_memoryCopy(pSiteMgr->hOs, pBuf + sizeof(dot11_eleHdr_t), (void *)pSsid->str, pSsid->len);
	size += sizeof(dot11_eleHdr_t) + pSsid->len;
	pBuf += sizeof(dot11_eleHdr_t) + pSsid->len;

	/* Rates */
	pDot11Rates = (dot11_RATES_t *) pBuf;

    /* 
     * Supported rates in probe request will always use the default rates for BG or A bands,
     * regardless of the STA desired rates.
     */
    if (radioBand == RADIO_BAND_2_4_GHZ)
	{
        /* Basic rates: 1,2,5.5,11 */  
		basicRateMask = rate_BasicToDrvBitmap(pSiteMgr->pDesiredParams->siteMgrRegstryBasicRate[DOT11_G_MODE], TI_FALSE);
        /* Extended: 6,9,12,18,24,36,48,54 */
        supportedRateMask = rate_SupportedToDrvBitmap(pSiteMgr->pDesiredParams->siteMgrRegstrySuppRate[DOT11_G_MODE], TI_FALSE);
    }
    else if (radioBand == RADIO_BAND_5_0_GHZ)
    {   /* Basic rates: 6,12,24 */
        basicRateMask = rate_BasicToDrvBitmap(pSiteMgr->pDesiredParams->siteMgrRegstryBasicRate[DOT11_A_MODE], TI_TRUE);
         /* Extended: 9,18,24,36,48,54 */
        supportedRateMask = rate_SupportedToDrvBitmap(pSiteMgr->pDesiredParams->siteMgrRegstrySuppRate[DOT11_A_MODE], TI_TRUE);
	}
	else
	{
        TRACE1(pSiteMgr->hReport, REPORT_SEVERITY_ERROR, "buildProbeReqTemplate, radioBand =%d ???\n",radioBand);
        /* Use default and pray for the best */
        /* Basic rates: 1,2,5.5,11 */  
        basicRateMask = rate_BasicToDrvBitmap(BASIC_RATE_SET_1_2_5_5_11, TI_FALSE);
        /* Extended: 6,9,12,18,24,36,48,54 */
        supportedRateMask = rate_SupportedToDrvBitmap(SUPPORTED_RATE_SET_UP_TO_54, TI_FALSE);
	}
	
	rate_DrvBitmapToNetStr (supportedRateMask, basicRateMask, ratesBuf, &len, &ofdmIndex);

TRACE5(pSiteMgr->hReport, REPORT_SEVERITY_INFORMATION, "buildProbeReqTemplate, supportedRateMask=0x%x, basicRateMask=0x%x, len=%d, ofdmIndex=%d, radioBand =%d\n",							 supportedRateMask,basicRateMask,len, ofdmIndex, radioBand);

       if(radioBand == RADIO_BAND_5_0_GHZ ||
       pSiteMgr->pDesiredParams->siteMgrUseDraftNum == DRAFT_5_AND_EARLIER ||
	   ofdmIndex == len)
	{
		pDot11Rates->hdr[0] = DOT11_SUPPORTED_RATES_ELE_ID;
		pDot11Rates->hdr[1] = len;
		os_memoryCopy(pSiteMgr->hOs, (void *)pDot11Rates->rates, ratesBuf, pDot11Rates->hdr[1]);
		size += pDot11Rates->hdr[1] + sizeof(dot11_eleHdr_t);
		pBuf += pDot11Rates->hdr[1] + sizeof(dot11_eleHdr_t);		
	}
	else
	{
		pDot11Rates->hdr[0] = DOT11_SUPPORTED_RATES_ELE_ID;
		pDot11Rates->hdr[1] = ofdmIndex;
		os_memoryCopy(pSiteMgr->hOs, (void *)pDot11Rates->rates, ratesBuf, pDot11Rates->hdr[1]);
		suppRatesLen = pDot11Rates->hdr[1] + sizeof(dot11_eleHdr_t);
		pDot11Rates = (dot11_RATES_t *) (pBuf + suppRatesLen); 
		pDot11Rates->hdr[0] = DOT11_EXT_SUPPORTED_RATES_ELE_ID;
		pDot11Rates->hdr[1] = len - ofdmIndex;
		os_memoryCopy(pSiteMgr->hOs, (void *)pDot11Rates->rates, &ratesBuf[ofdmIndex], pDot11Rates->hdr[1]);
		extSuppRatesLen = pDot11Rates->hdr[1] + sizeof(dot11_eleHdr_t);
		size += suppRatesLen + extSuppRatesLen;
		pBuf += suppRatesLen + extSuppRatesLen;		
	}


    /* add HT capabilities IE */
    StaCap_GetHtCapabilitiesIe (pSiteMgr->hStaCap, pBuf, &len);
    size += len;
    pBuf += len;


	/* WiFi Simple Config */
	if (pSiteMgr->includeWSCinProbeReq)
    {
	if(pSiteMgr->siteMgrWSCCurrMode != TIWLN_SIMPLE_CONFIG_OFF)
	{
		((dot11_WSC_t *)(pBuf))->hdr[0] = DOT11_WSC_PARAM_ELE_ID;
		 ((dot11_WSC_t *)(pBuf))->hdr[1] = DOT11_WSC_PROBE_REQ_MAX_LENGTH + DOT11_OUI_LEN + 1;
        pBuf += sizeof(dot11_eleHdr_t);
         os_memoryCopy(pSiteMgr->hOs, pBuf, &WSCOuiIe, DOT11_OUI_LEN+2);
		 os_memoryCopy(pSiteMgr->hOs, pBuf + DOT11_OUI_LEN+1, &pSiteMgr->siteMgrWSCProbeReqParams, DOT11_WSC_PROBE_REQ_MAX_LENGTH - (DOT11_OUI_LEN+2));
		 size += sizeof(dot11_eleHdr_t) + DOT11_WSC_PROBE_REQ_MAX_LENGTH + DOT11_OUI_LEN + 1;
		 pBuf += sizeof(dot11_eleHdr_t) + DOT11_WSC_PROBE_REQ_MAX_LENGTH + DOT11_OUI_LEN + 1;	
	  }
	}
	pTemplate->len = size;
	
	return TI_OK;
}

/************************************************************************
 *                        buildProbeRspTemplate							*
 ************************************************************************
DESCRIPTION: This function build a probe response template to set to the HAL 
				when joining an IBSS network.
				performs the following:
				-	Build a template & set the template len, the template type is set in the site mgr
				-	The template is built based on the chosen site attributes

			NOTE: This function is used to build beacon template too.
			The site manager set the template type (after thos function returns) to beacon or probe response accordingly.
                                                                                                   
INPUT:      pSiteMgr	-	Handle to site manager	
			pTemplate	-	Pointer to the template structure		


OUTPUT:		


RETURN:     TI_OK

************************************************************************/
TI_STATUS buildProbeRspTemplate(siteMgr_t *pSiteMgr, TSetTemplate *pTemplate)
{
	paramInfo_t			param;
	TI_UINT8			*pBuf;
	probeRspTemplate_t	*pBuffer = (probeRspTemplate_t	*)pTemplate->ptr;
	siteEntry_t			*pPrimarySite = pSiteMgr->pSitesMgmtParams->pPrimarySite;
	TI_INT32			i, j;
	TI_UINT32			size;
	dot11_RATES_t		*pDot11Rates;
	dot11_ERP_t         *pdot11Erp;
	TI_UINT32			len = 0, ofdmIndex = 0;
	TI_BOOL				extRates = TI_FALSE;
	TI_BOOL             useProtection,NonErpPresent,barkerPreambleType;
	TCountry			*pCountry = NULL;
	TI_UINT8			ratesBuf[DOT11_MAX_SUPPORTED_RATES];
	TI_UINT32			supportedRateMask,basicRateMask;
	TI_UINT16			headerFC = DOT11_FC_PROBE_RESP;

	os_memoryZero(pSiteMgr->hOs, pBuffer, sizeof(probeRspTemplate_t));


	/*
	 * Build WLAN Header:
	 * ==================
	 */

	/* Set destination address */
	for (i = 0; i < MAC_ADDR_LEN; i++)
		pBuffer->hdr.DA[i] = 0xFF;

	/* Set BSSID address */
	MAC_COPY (pBuffer->hdr.BSSID, pPrimarySite->bssid);  

	/* Build Source address */
	param.paramType = CTRL_DATA_MAC_ADDRESS;
	ctrlData_getParam(pSiteMgr->hCtrlData, &param);
	MAC_COPY (pBuffer->hdr.SA, param.content.ctrlDataDeviceMacAddress);  
	
    COPY_WLAN_WORD(&pBuffer->hdr.fc, &headerFC);

	size = sizeof(dot11_mgmtHeader_t);
	pBuf = (TI_UINT8 *)pBuffer->timeStamp;
   /*
	* Fixed Fields
	*/
	/* we skip the timestamp field */
	size += TIME_STAMP_LEN;
	pBuf += TIME_STAMP_LEN;

	/* Beacon interval */
    COPY_WLAN_WORD(pBuf, &pPrimarySite->beaconInterval);
	size += FIX_FIELD_LEN;
	pBuf += FIX_FIELD_LEN;

	/* capabilities */
    COPY_WLAN_WORD(pBuf, &pPrimarySite->capabilities);
	size += FIX_FIELD_LEN;
	pBuf += FIX_FIELD_LEN;

	/*
	 * Build Informataion Elements:
	 * ============================
	 */

	/* SSID IE */
	((dot11_SSID_t *)(pBuf))->hdr[0] = DOT11_SSID_ELE_ID;
	((dot11_SSID_t *)(pBuf))->hdr[1] = pPrimarySite->ssid.len;
	os_memoryCopy(pSiteMgr->hOs, pBuf + sizeof(dot11_eleHdr_t), (void *)pPrimarySite->ssid.str, pPrimarySite->ssid.len);
	size += sizeof(dot11_eleHdr_t) + pPrimarySite->ssid.len;
	pBuf += sizeof(dot11_eleHdr_t) + pPrimarySite->ssid.len;


	/* Rates IE */

	pDot11Rates = (dot11_RATES_t *) pBuf;

	if (pPrimarySite->channel == SPECIAL_BG_CHANNEL) 
	{
		supportedRateMask = rate_GetDrvBitmapForDefaultSupporteSet ();
		basicRateMask	  = rate_GetDrvBitmapForDefaultBasicSet ();
	}
	else
	{
		supportedRateMask = pSiteMgr->pDesiredParams->siteMgrMatchedSuppRateMask;
		basicRateMask     = pSiteMgr->pDesiredParams->siteMgrMatchedBasicRateMask;
	}
	
	rate_DrvBitmapToNetStr (supportedRateMask, basicRateMask, ratesBuf, &len, &ofdmIndex);

    if(pSiteMgr->siteMgrOperationalMode != DOT11_G_MODE ||
       pSiteMgr->pDesiredParams->siteMgrUseDraftNum == DRAFT_5_AND_EARLIER ||
	   ofdmIndex == len)
	{
		pDot11Rates->hdr[0] = DOT11_SUPPORTED_RATES_ELE_ID;
		pDot11Rates->hdr[1] = len;
		os_memoryCopy(pSiteMgr->hOs, (void *)pDot11Rates->rates, ratesBuf, pDot11Rates->hdr[1]);
		size += pDot11Rates->hdr[1] + sizeof(dot11_eleHdr_t);
		pBuf += pDot11Rates->hdr[1] + sizeof(dot11_eleHdr_t);		
	}
	else
	{
		pDot11Rates->hdr[0] = DOT11_SUPPORTED_RATES_ELE_ID;
		pDot11Rates->hdr[1] = ofdmIndex;
		os_memoryCopy(pSiteMgr->hOs, (void *)pDot11Rates->rates, ratesBuf, pDot11Rates->hdr[1]);
		size += pDot11Rates->hdr[1] + sizeof(dot11_eleHdr_t);
		pBuf += pDot11Rates->hdr[1] + sizeof(dot11_eleHdr_t);
		extRates = TI_TRUE;
	}

	/* DS IE */
	((dot11_DS_PARAMS_t *)(pBuf))->hdr[0] = DOT11_DS_PARAMS_ELE_ID;
	((dot11_DS_PARAMS_t *)(pBuf))->hdr[1] = DOT11_DS_PARAMS_ELE_LEN;
	((dot11_DS_PARAMS_t *)(pBuf))->currChannel = pPrimarySite->channel;
	size += sizeof(dot11_eleHdr_t) + DOT11_DS_PARAMS_ELE_LEN;
	pBuf += sizeof(dot11_eleHdr_t) + DOT11_DS_PARAMS_ELE_LEN;

	/* IBSS IE */
	((dot11_IBSS_PARAMS_t *)(pBuf))->hdr[0] = DOT11_IBSS_PARAMS_ELE_ID;
	((dot11_IBSS_PARAMS_t *)(pBuf))->hdr[1] = DOT11_IBSS_PARAMS_ELE_LEN;
	COPY_WLAN_WORD(&((dot11_IBSS_PARAMS_t *)(pBuf))->atimWindow, &pPrimarySite->atimWindow);
	size += sizeof(dot11_eleHdr_t) + DOT11_IBSS_PARAMS_ELE_LEN;
	pBuf += sizeof(dot11_eleHdr_t) + DOT11_IBSS_PARAMS_ELE_LEN;

	/* Country IE */
	param.paramType = REGULATORY_DOMAIN_ENABLED_PARAM;
	regulatoryDomain_getParam(pSiteMgr->hRegulatoryDomain,&param);

	if(	param.content.regulatoryDomainEnabled == TI_TRUE )
	{
        /* get country IE */
        param.paramType = REGULATORY_DOMAIN_COUNTRY_PARAM;
		regulatoryDomain_getParam(pSiteMgr->hRegulatoryDomain, &param);
		pCountry = param.content.pCountry;

        /* Check if a country IE was found */
		if(pCountry != NULL)
		{
			*pBuf = DOT11_COUNTRY_ELE_ID;
			pBuf++;
			size++;
			*pBuf = (TI_UINT8)(pCountry->len);
			pBuf++;
			size++;
			
			/* Note: The country structure is not byte-aligned so it is copied as follows to ensure
			           that there are no gaps in the output structure (pBuf). */

			os_memoryCopy(pSiteMgr->hOs, pBuf , &pCountry->countryIE.CountryString, DOT11_COUNTRY_STRING_LEN);
			pBuf += DOT11_COUNTRY_STRING_LEN;
			size += DOT11_COUNTRY_STRING_LEN;

			/* Loop on all tripletChannels. Each item has three fields ('i' counts rows and 'j' counts bytes). */
			for (i = 0, j = 0;  j < (pCountry->len - DOT11_COUNTRY_STRING_LEN);  i++, j+=3)
			{
				*(pBuf + j    ) = pCountry->countryIE.tripletChannels[i].firstChannelNumber;
				*(pBuf + j + 1) = pCountry->countryIE.tripletChannels[i].maxTxPowerLevel;
				*(pBuf + j + 2) = pCountry->countryIE.tripletChannels[i].numberOfChannels;
			}

			pBuf += (pCountry->len - DOT11_COUNTRY_STRING_LEN);
			size += (pCountry->len - DOT11_COUNTRY_STRING_LEN);
		}
	}
	 
	/*ERP IE*/
	siteMgr_IsERP_Needed(pSiteMgr,&useProtection,&NonErpPresent,&barkerPreambleType);
	if (useProtection || NonErpPresent || barkerPreambleType)
	{
		pdot11Erp = (dot11_ERP_t *) pBuf;
		pdot11Erp->hdr[0] = DOT11_ERP_IE_ID;
		pdot11Erp->hdr[1] = 1;
		pdot11Erp->ctrl = 0;
		if (NonErpPresent)
			pdot11Erp->ctrl |= ERP_IE_NON_ERP_PRESENT_MASK;
		if (useProtection)
			pdot11Erp->ctrl |= ERP_IE_USE_PROTECTION_MASK;
		if (barkerPreambleType)
			pdot11Erp->ctrl |= ERP_IE_BARKER_PREAMBLE_MODE_MASK;
		size += pdot11Erp->hdr[1] + sizeof(dot11_eleHdr_t);
		pBuf += pdot11Erp->hdr[1] + sizeof(dot11_eleHdr_t);
		
	}


	/* Extended supported rates IE */
	if(extRates)
	{
		pDot11Rates = (dot11_RATES_t *) pBuf;
		pDot11Rates->hdr[0] = DOT11_EXT_SUPPORTED_RATES_ELE_ID;
		pDot11Rates->hdr[1] = len - ofdmIndex;
		os_memoryCopy(pSiteMgr->hOs, (void *)pDot11Rates->rates, &ratesBuf[ofdmIndex], pDot11Rates->hdr[1]);
		size += pDot11Rates->hdr[1] + sizeof(dot11_eleHdr_t);
		pBuf += pDot11Rates->hdr[1] + sizeof(dot11_eleHdr_t);	
	}

    /* no need to insert RSN information elements */
		 
	pTemplate->len = size;
TRACE1(pSiteMgr->hReport, REPORT_SEVERITY_INFORMATION, "Probe response template len = %d\n",size);
	
	return TI_OK;
}

/************************************************************************
 *                        buildPsPollTemplate							*
 ************************************************************************
DESCRIPTION: This function build a ps poll template
				performs the following:
				-	Build a template & set the template len, the template type is set in the site mgr
                                                                                                   
INPUT:      pSiteMgr	-	Handle to site manager	
			pTemplate	-	Pointer to the template structure		
			pSsid		-	Desired SSID

OUTPUT:		

RETURN:     TI_OK
************************************************************************/
TI_STATUS buildPsPollTemplate(siteMgr_t *pSiteMgr, TSetTemplate *pTemplate)
{
    paramInfo_t			param;
    TTwdParamInfo       tTwdParam;
	TI_UINT32				size;
	psPollTemplate_t	*pBuffer = (psPollTemplate_t *)pTemplate->ptr;
	siteEntry_t			*pPrimarySite = pSiteMgr->pSitesMgmtParams->pPrimarySite;
	TI_UINT16				fc;

	os_memoryZero(pSiteMgr->hOs, pBuffer, sizeof(psPollTemplate_t));

	/*
	 * Header First
	 */
	
	/* Set BSSID address */
	MAC_COPY (pBuffer->hdr.BSSID, pPrimarySite->bssid);  

	/* Build Source address */
	param.paramType = CTRL_DATA_MAC_ADDRESS;
	ctrlData_getParam(pSiteMgr->hCtrlData, &param);
	MAC_COPY (pBuffer->hdr.TA, param.content.ctrlDataDeviceMacAddress);  
	
    /*
    **   Building the Frame Control word (16 bits)
    ** ---------------------------------------------
    ** Type = Control
    ** SubType = Power Save (PS) POLL,  */
    fc = DOT11_FC_PS_POLL;
    /*
    ** setting the Power Management bit in the Frame control field
    ** to be "Power Save mode"
    */
    fc |= (0x1 << DOT11_FC_PWR_MGMT_SHIFT);

	COPY_WLAN_WORD(&pBuffer->hdr.fc, &fc); /* copy with endianess handling. */

    /*
    **   Association ID
    ** -----------------
    */
    tTwdParam.paramType = TWD_AID_PARAM_ID;
    TWD_GetParam (pSiteMgr->hTWD, &tTwdParam);

    /* AID should have its two MSB bit Set to "1"*/
    pBuffer->hdr.AID = tTwdParam.content.halCtrlAid | 0xC000;

	size = sizeof(dot11_PsPollFrameHeader_t);

	pTemplate->len = size;

	return TI_OK;
}


/************************************************************************
 *                        buildQosNullDataTemplate							*
 ************************************************************************
DESCRIPTION: This function build a qos null data template
				performs the following:
				-	Build a template & set the template len, the template type is set in the site mgr
                                                                                                   
INPUT:      pSiteMgr	-	Handle to site manager	
			pTemplate	-	Pointer to the template structure		
			pSsid		-	Desired SSID

OUTPUT:		

RETURN:     TI_OK
************************************************************************/
TI_STATUS buildQosNullDataTemplate(siteMgr_t *pSiteMgr, TSetTemplate *pTemplate, TI_UINT8 userPriority)
{
	paramInfo_t			param;
	TI_UINT32				size;
	QosNullDataTemplate_t	*pBuffer = (QosNullDataTemplate_t	*)pTemplate->ptr;
	siteEntry_t			*pPrimarySite = pSiteMgr->pSitesMgmtParams->pPrimarySite;
	TI_UINT16				fc;
	TI_UINT16				qosControl;

	os_memoryZero(pSiteMgr->hOs, pBuffer, sizeof(QosNullDataTemplate_t));

	/*
	 * Header First
	 */
	/* Set destination address */
    if (pPrimarySite)
    {
	  MAC_COPY (pBuffer->hdr.address1, pPrimarySite->bssid);  

	  /* Set BSSID address */
	  MAC_COPY (pBuffer->hdr.address3, pPrimarySite->bssid);  
    }
    else
    {
TRACE0(pSiteMgr->hReport, REPORT_SEVERITY_INFORMATION, "No Primary site so cannot fill QosNullData template\n");
    }

	/* Build Source address */
	param.paramType = CTRL_DATA_MAC_ADDRESS;
	ctrlData_getParam(pSiteMgr->hCtrlData, &param);
	MAC_COPY (pBuffer->hdr.address2, param.content.ctrlDataDeviceMacAddress);  

	fc = DOT11_FC_DATA_NULL_QOS | (1 << DOT11_FC_TO_DS_SHIFT);
	COPY_WLAN_WORD(&pBuffer->hdr.fc, &fc); /* copy with endianess handling. */

    qosControl = (TI_UINT16)userPriority;
	qosControl <<= QOS_CONTROL_UP_SHIFT;
	COPY_WLAN_WORD(&pBuffer->hdr.qosControl, &qosControl); /* copy with endianess handling. */

	size = WLAN_QOS_HDR_LEN;

	pTemplate->len = size;
	
	return TI_OK;    
}
   
   
   
   
   
