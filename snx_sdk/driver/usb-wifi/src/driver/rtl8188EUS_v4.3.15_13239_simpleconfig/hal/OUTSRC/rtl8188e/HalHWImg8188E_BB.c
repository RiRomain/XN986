/****************************************************************************** 
* 
* Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved. 
* 
* This program is free software; you can redistribute it and/or modify it 
* under the terms of version 2 of the GNU General Public License as 
* published by the Free Software Foundation. 
* 
* This program is distributed in the hope that it will be useful, but WITHOUT 
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for 
* more details. 
* 
* You should have received a copy of the GNU General Public License along with 
* this program; if not, write to the Free Software Foundation, Inc., 
* 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA 
* 
* 
******************************************************************************/

#include "Mp_Precomp.h"
#include "../phydm_precomp.h"

#if (RTL8188E_SUPPORT == 1)
static BOOLEAN
CheckPositive(
    IN  PDM_ODM_T     pDM_Odm,
    IN  const u4Byte  Condition1,
    IN  const u4Byte  Condition2
    )
{
    u1Byte    _BoardType = ((pDM_Odm->BoardType & BIT4) >> 4) << 0 | // _GLNA
                           ((pDM_Odm->BoardType & BIT3) >> 3) << 1 | // _GPA 
                           ((pDM_Odm->BoardType & BIT7) >> 7) << 2 | // _ALNA
                           ((pDM_Odm->BoardType & BIT6) >> 6) << 3 | // _APA 
                           ((pDM_Odm->BoardType & BIT2) >> 2) << 4;  // _BT  

	u4Byte 	  cond1   = Condition1, cond2 = Condition2;
	u4Byte    driver1 = pDM_Odm->CutVersion       << 24 |  
		                pDM_Odm->SupportPlatform  << 16 | 
		                pDM_Odm->PackageType      << 12 | 
		                pDM_Odm->SupportInterface << 8  |
		                _BoardType;

	u4Byte    driver2 = pDM_Odm->TypeGLNA <<  0 |  
		                pDM_Odm->TypeGPA  <<  8 | 
		                pDM_Odm->TypeALNA << 16 | 
		                pDM_Odm->TypeAPA  << 24; 

    ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_TRACE, 
                ("===> [8812A] CheckPositive (cond1, cond2) = (0x%X 0x%X)\n", cond1, cond2));
    ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_TRACE, 
                ("===> [8812A] CheckPositive (driver1, driver2) = (0x%X 0x%X)\n", driver1, driver2));

    ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_TRACE, 
                ("	(Platform, Interface) = (0x%X, 0x%X)\n", pDM_Odm->SupportPlatform, pDM_Odm->SupportInterface));
    ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_TRACE, 
                ("	(Board, Package) = (0x%X, 0x%X)\n", pDM_Odm->BoardType, pDM_Odm->PackageType));


	//============== Value Defined Check ===============//
	//QFN Type [15:12] and Cut Version [27:24] need to do value check
	
	if(((cond1 & 0x0000F000) != 0) &&((cond1 & 0x0000F000) != (driver1 & 0x0000F000)))
		return FALSE;
	if(((cond1 & 0x0F000000) != 0) &&((cond1 & 0x0F000000) != (driver1 & 0x0F000000)))
		return FALSE;		

	//=============== Bit Defined Check ================//
    // We don't care [31:28] and [23:20]
    //
	cond1   &= 0x000F0FFF; 
	driver1 &= 0x000F0FFF; 

    if ((cond1 & driver1) == cond1) 
    {
        u4Byte bitMask = 0;
        if ((cond1 & 0x0F) == 0) // BoardType is DONTCARE
            return TRUE;

        if ((cond1 & BIT0) != 0) //GLNA
            bitMask |= 0x000000FF;
        if ((cond1 & BIT1) != 0) //GPA
            bitMask |= 0x0000FF00;
        if ((cond1 & BIT2) != 0) //ALNA
            bitMask |= 0x00FF0000;
        if ((cond1 & BIT3) != 0) //APA
            bitMask |= 0xFF000000;

        if ((cond2 & bitMask) == (driver2 & bitMask)) // BoardType of each RF path is matched
            return TRUE;
        else
            return FALSE;
    }
    else 
    {
        return FALSE;
    }
}
static BOOLEAN
CheckNegative(
    IN  PDM_ODM_T     pDM_Odm,
    IN  const u4Byte  Condition1,
    IN  const u4Byte  Condition2
    )
{
    return TRUE;
}

/******************************************************************************
*                           AGC_TAB.TXT
******************************************************************************/

u4Byte Array_MP_8188E_AGC_TAB[] = { 
	0x88000001,0x00000000,0x40000000,0x00000000,
		0xC78, 0xF6000001,
		0xC78, 0xF5010001,
		0xC78, 0xF4020001,
		0xC78, 0xF3030001,
		0xC78, 0xF2040001,
	0x90000001,0x00000000,0x40000000,0x00000000,
		0xC78, 0xF7000001,
		0xC78, 0xF6010001,
		0xC78, 0xF5020001,
		0xC78, 0xF4030001,
		0xC78, 0xF3040001,
	0xA0000000,0x00000000,
		0xC78, 0xFB000001,
		0xC78, 0xFB010001,
		0xC78, 0xFB020001,
		0xC78, 0xFB030001,
		0xC78, 0xFB040001,
	0xB0000000,0x00000000,
	0x88000001,0x00000000,0x40000000,0x00000000,
		0xC78, 0xF1050001,
		0xC78, 0xF0060001,
		0xC78, 0xEF070001,
		0xC78, 0xEE080001,
		0xC78, 0xED090001,
		0xC78, 0xEC0A0001,
		0xC78, 0xEB0B0001,
		0xC78, 0xEA0C0001,
		0xC78, 0xE90D0001,
		0xC78, 0xE80E0001,
		0xC78, 0xE70F0001,
		0xC78, 0xE6100001,
		0xC78, 0xE5110001,
		0xC78, 0xE4120001,
		0xC78, 0xE3130001,
		0xC78, 0xE2140001,
		0xC78, 0xC5150001,
		0xC78, 0xC4160001,
		0xC78, 0xC3170001,
		0xC78, 0xC2180001,
		0xC78, 0x88190001,
		0xC78, 0x871A0001,
		0xC78, 0x861B0001,
		0xC78, 0x851C0001,
		0xC78, 0x841D0001,
		0xC78, 0x831E0001,
		0xC78, 0x821F0001,
		0xC78, 0x81200001,
		0xC78, 0x80210001,
	0x90000001,0x00000000,0x40000000,0x00000000,
		0xC78, 0xF2050001,
		0xC78, 0xF1060001,
		0xC78, 0xF0070001,
		0xC78, 0xEF080001,
		0xC78, 0xEE090001,
		0xC78, 0xED0A0001,
		0xC78, 0xEC0B0001,
		0xC78, 0xEB0C0001,
		0xC78, 0xEA0D0001,
		0xC78, 0xE90E0001,
		0xC78, 0xE80F0001,
		0xC78, 0xE7100001,
		0xC78, 0xE6110001,
		0xC78, 0xE5120001,
		0xC78, 0xE4130001,
		0xC78, 0xE3140001,
		0xC78, 0xE2150001,
		0xC78, 0xE1160001,
		0xC78, 0x89170001,
		0xC78, 0x88180001,
		0xC78, 0x87190001,
		0xC78, 0x861A0001,
		0xC78, 0x851B0001,
		0xC78, 0x841C0001,
		0xC78, 0x831D0001,
		0xC78, 0x821E0001,
		0xC78, 0x811F0001,
		0xC78, 0x6B200001,
		0xC78, 0x6A210001,
	0x98000000,0x00000000,0x40000000,0x00000000,
		0xC78, 0xFA050001,
		0xC78, 0xF9060001,
		0xC78, 0xF8070001,
		0xC78, 0xF7080001,
		0xC78, 0xF6090001,
		0xC78, 0xF50A0001,
		0xC78, 0xF40B0001,
		0xC78, 0xF30C0001,
		0xC78, 0xF20D0001,
		0xC78, 0xF10E0001,
		0xC78, 0xF00F0001,
		0xC78, 0xEF100001,
		0xC78, 0xEE110001,
		0xC78, 0xED120001,
		0xC78, 0xEC130001,
		0xC78, 0xEB140001,
		0xC78, 0xEA150001,
		0xC78, 0xE9160001,
		0xC78, 0xE8170001,
		0xC78, 0xE7180001,
		0xC78, 0xE6190001,
		0xC78, 0xE51A0001,
		0xC78, 0xE41B0001,
		0xC78, 0xC71C0001,
		0xC78, 0xC61D0001,
		0xC78, 0xC51E0001,
		0xC78, 0xC41F0001,
		0xC78, 0xC3200001,
		0xC78, 0xC2210001,
	0xA0000000,0x00000000,
		0xC78, 0xFB050001,
		0xC78, 0xFA060001,
		0xC78, 0xF9070001,
		0xC78, 0xF8080001,
		0xC78, 0xF7090001,
		0xC78, 0xF60A0001,
		0xC78, 0xF50B0001,
		0xC78, 0xF40C0001,
		0xC78, 0xF30D0001,
		0xC78, 0xF20E0001,
		0xC78, 0xF10F0001,
		0xC78, 0xF0100001,
		0xC78, 0xEF110001,
		0xC78, 0xEE120001,
		0xC78, 0xED130001,
		0xC78, 0xEC140001,
		0xC78, 0xEB150001,
		0xC78, 0xEA160001,
		0xC78, 0xE9170001,
		0xC78, 0xE8180001,
		0xC78, 0xE7190001,
		0xC78, 0xE61A0001,
		0xC78, 0xE51B0001,
		0xC78, 0xE41C0001,
		0xC78, 0xE31D0001,
		0xC78, 0xE21E0001,
		0xC78, 0xE11F0001,
		0xC78, 0x8A200001,
		0xC78, 0x89210001,
	0xB0000000,0x00000000,
	0x88000001,0x00000000,0x40000000,0x00000000,
		0xC78, 0x66220001,
		0xC78, 0x65230001,
		0xC78, 0x64240001,
		0xC78, 0x63250001,
		0xC78, 0x62260001,
		0xC78, 0x61270001,
		0xC78, 0x60280001,
	0x90000001,0x00000000,0x40000000,0x00000000,
		0xC78, 0x69220001,
		0xC78, 0x68230001,
		0xC78, 0x67240001,
		0xC78, 0x66250001,
		0xC78, 0x65260001,
		0xC78, 0x64270001,
		0xC78, 0x63280001,
	0xA0000000,0x00000000,
		0xC78, 0x88220001,
		0xC78, 0x87230001,
		0xC78, 0x86240001,
		0xC78, 0x85250001,
		0xC78, 0x84260001,
		0xC78, 0x83270001,
		0xC78, 0x82280001,
	0xB0000000,0x00000000,
	0x88000001,0x00000000,0x40000000,0x00000000,
		0xC78, 0x4A290001,
		0xC78, 0x492A0001,
		0xC78, 0x482B0001,
		0xC78, 0x472C0001,
	0x90000001,0x00000000,0x40000000,0x00000000,
		0xC78, 0x62290001,
		0xC78, 0x612A0001,
		0xC78, 0x462B0001,
		0xC78, 0x452C0001,
	0x98000000,0x00000000,0x40000000,0x00000000,
		0xC78, 0x81290001,
		0xC78, 0x242A0001,
		0xC78, 0x232B0001,
		0xC78, 0x222C0001,
	0xA0000000,0x00000000,
		0xC78, 0x6B290001,
		0xC78, 0x6A2A0001,
		0xC78, 0x692B0001,
		0xC78, 0x682C0001,
	0xB0000000,0x00000000,
	0x88000001,0x00000000,0x40000000,0x00000000,
		0xC78, 0x462D0001,
		0xC78, 0x452E0001,
		0xC78, 0x442F0001,
		0xC78, 0x43300001,
		0xC78, 0x42310001,
		0xC78, 0x41320001,
		0xC78, 0x40330001,
	0x90000001,0x00000000,0x40000000,0x00000000,
		0xC78, 0x442D0001,
		0xC78, 0x432E0001,
		0xC78, 0x422F0001,
		0xC78, 0x41300001,
		0xC78, 0x40310001,
		0xC78, 0x40320001,
		0xC78, 0x40330001,
	0xA0000000,0x00000000,
		0xC78, 0x672D0001,
		0xC78, 0x662E0001,
		0xC78, 0x652F0001,
		0xC78, 0x64300001,
		0xC78, 0x63310001,
		0xC78, 0x62320001,
		0xC78, 0x61330001,
	0xB0000000,0x00000000,
	0x88000001,0x00000000,0x40000000,0x00000000,
		0xC78, 0x40340001,
		0xC78, 0x40350001,
		0xC78, 0x40360001,
		0xC78, 0x40370001,
		0xC78, 0x40380001,
		0xC78, 0x40390001,
		0xC78, 0x403A0001,
		0xC78, 0x403B0001,
		0xC78, 0x403C0001,
		0xC78, 0x403D0001,
		0xC78, 0x403E0001,
	0x90000001,0x00000000,0x40000000,0x00000000,
		0xC78, 0x40340001,
		0xC78, 0x40350001,
		0xC78, 0x40360001,
		0xC78, 0x40370001,
		0xC78, 0x40380001,
		0xC78, 0x40390001,
		0xC78, 0x403A0001,
		0xC78, 0x403B0001,
		0xC78, 0x403C0001,
		0xC78, 0x403D0001,
		0xC78, 0x403E0001,
	0x98000000,0x00000000,0x40000000,0x00000000,
		0xC78, 0x60340001,
		0xC78, 0x4A350001,
		0xC78, 0x49360001,
		0xC78, 0x48370001,
		0xC78, 0x47380001,
		0xC78, 0x46390001,
		0xC78, 0x453A0001,
		0xC78, 0x443B0001,
		0xC78, 0x433C0001,
		0xC78, 0x423D0001,
		0xC78, 0x413E0001,
	0xA0000000,0x00000000,
		0xC78, 0x46340001,
		0xC78, 0x45350001,
		0xC78, 0x44360001,
		0xC78, 0x43370001,
		0xC78, 0x42380001,
		0xC78, 0x41390001,
		0xC78, 0x403A0001,
		0xC78, 0x403B0001,
		0xC78, 0x403C0001,
		0xC78, 0x403D0001,
		0xC78, 0x403E0001,
	0xB0000000,0x00000000,
		0xC78, 0x403F0001,
	0x88000001,0x00000000,0x40000000,0x00000000,
		0xC78, 0xFB400001,
		0xC78, 0xFA410001,
		0xC78, 0xF9420001,
		0xC78, 0xF8430001,
		0xC78, 0xF7440001,
		0xC78, 0xF6450001,
		0xC78, 0xF5460001,
		0xC78, 0xF4470001,
		0xC78, 0xF3480001,
		0xC78, 0xF2490001,
		0xC78, 0xF14A0001,
		0xC78, 0xF04B0001,
		0xC78, 0xEF4C0001,
		0xC78, 0xEE4D0001,
		0xC78, 0xED4E0001,
		0xC78, 0xEC4F0001,
		0xC78, 0xEB500001,
		0xC78, 0xEA510001,
		0xC78, 0xE9520001,
		0xC78, 0xE8530001,
		0xC78, 0xE7540001,
		0xC78, 0xE6550001,
		0xC78, 0xE5560001,
		0xC78, 0xC6570001,
		0xC78, 0xC5580001,
		0xC78, 0xC4590001,
		0xC78, 0xC35A0001,
		0xC78, 0xC25B0001,
		0xC78, 0xC15C0001,
		0xC78, 0xC05D0001,
		0xC78, 0xA35E0001,
		0xC78, 0xA25F0001,
		0xC78, 0xA1600001,
		0xC78, 0x88610001,
		0xC78, 0x87620001,
		0xC78, 0x86630001,
		0xC78, 0x85640001,
		0xC78, 0x84650001,
		0xC78, 0x83660001,
		0xC78, 0x82670001,
	0x90000001,0x00000000,0x40000000,0x00000000,
		0xC78, 0xFB400001,
		0xC78, 0xFA410001,
		0xC78, 0xF9420001,
		0xC78, 0xF8430001,
		0xC78, 0xF7440001,
		0xC78, 0xF6450001,
		0xC78, 0xF5460001,
		0xC78, 0xF4470001,
		0xC78, 0xF3480001,
		0xC78, 0xF2490001,
		0xC78, 0xF14A0001,
		0xC78, 0xF04B0001,
		0xC78, 0xEF4C0001,
		0xC78, 0xEE4D0001,
		0xC78, 0xED4E0001,
		0xC78, 0xEC4F0001,
		0xC78, 0xEB500001,
		0xC78, 0xEA510001,
		0xC78, 0xE9520001,
		0xC78, 0xE8530001,
		0xC78, 0xE7540001,
		0xC78, 0xE6550001,
		0xC78, 0xE5560001,
		0xC78, 0xE4570001,
		0xC78, 0xE3580001,
		0xC78, 0xE2590001,
		0xC78, 0xC35A0001,
		0xC78, 0xC25B0001,
		0xC78, 0xC15C0001,
		0xC78, 0x8B5D0001,
		0xC78, 0x8A5E0001,
		0xC78, 0x895F0001,
		0xC78, 0x88600001,
		0xC78, 0x87610001,
		0xC78, 0x86620001,
		0xC78, 0x85630001,
		0xC78, 0x84640001,
		0xC78, 0x67650001,
		0xC78, 0x66660001,
		0xC78, 0x65670001,
	0x98000000,0x00000000,0x40000000,0x00000000,
		0xC78, 0xFB400001,
		0xC78, 0xFB410001,
		0xC78, 0xFB420001,
		0xC78, 0xFB430001,
		0xC78, 0xFB440001,
		0xC78, 0xFB450001,
		0xC78, 0xFB460001,
		0xC78, 0xFB470001,
		0xC78, 0xFA480001,
		0xC78, 0xF9490001,
		0xC78, 0xF84A0001,
		0xC78, 0xF74B0001,
		0xC78, 0xF64C0001,
		0xC78, 0xF54D0001,
		0xC78, 0xF44E0001,
		0xC78, 0xF34F0001,
		0xC78, 0xF2500001,
		0xC78, 0xF1510001,
		0xC78, 0xF0520001,
		0xC78, 0xEF530001,
		0xC78, 0xEE540001,
		0xC78, 0xED550001,
		0xC78, 0xEC560001,
		0xC78, 0xEB570001,
		0xC78, 0xEA580001,
		0xC78, 0xE9590001,
		0xC78, 0xE85A0001,
		0xC78, 0xE75B0001,
		0xC78, 0xE65C0001,
		0xC78, 0xE55D0001,
		0xC78, 0xC65E0001,
		0xC78, 0xC55F0001,
		0xC78, 0xC4600001,
		0xC78, 0xC3610001,
		0xC78, 0xC2620001,
		0xC78, 0xC1630001,
		0xC78, 0xC0640001,
		0xC78, 0xA3650001,
		0xC78, 0xA2660001,
		0xC78, 0xA1670001,
	0xA0000000,0x00000000,
		0xC78, 0xFB400001,
		0xC78, 0xFB410001,
		0xC78, 0xFB420001,
		0xC78, 0xFB430001,
		0xC78, 0xFB440001,
		0xC78, 0xFB450001,
		0xC78, 0xFB460001,
		0xC78, 0xFB470001,
		0xC78, 0xFB480001,
		0xC78, 0xFA490001,
		0xC78, 0xF94A0001,
		0xC78, 0xF84B0001,
		0xC78, 0xF74C0001,
		0xC78, 0xF64D0001,
		0xC78, 0xF54E0001,
		0xC78, 0xF44F0001,
		0xC78, 0xF3500001,
		0xC78, 0xF2510001,
		0xC78, 0xF1520001,
		0xC78, 0xF0530001,
		0xC78, 0xEF540001,
		0xC78, 0xEE550001,
		0xC78, 0xED560001,
		0xC78, 0xEC570001,
		0xC78, 0xEB580001,
		0xC78, 0xEA590001,
		0xC78, 0xE95A0001,
		0xC78, 0xE85B0001,
		0xC78, 0xE75C0001,
		0xC78, 0xE65D0001,
		0xC78, 0xE55E0001,
		0xC78, 0xE45F0001,
		0xC78, 0xE3600001,
		0xC78, 0xE2610001,
		0xC78, 0xC3620001,
		0xC78, 0xC2630001,
		0xC78, 0xC1640001,
		0xC78, 0x8B650001,
		0xC78, 0x8A660001,
		0xC78, 0x89670001,
	0xB0000000,0x00000000,
	0x88000001,0x00000000,0x40000000,0x00000000,
		0xC78, 0x66680001,
		0xC78, 0x65690001,
		0xC78, 0x646A0001,
		0xC78, 0x636B0001,
		0xC78, 0x626C0001,
	0x90000001,0x00000000,0x40000000,0x00000000,
		0xC78, 0x64680001,
		0xC78, 0x63690001,
		0xC78, 0x626A0001,
		0xC78, 0x616B0001,
		0xC78, 0x606C0001,
	0xA0000000,0x00000000,
		0xC78, 0x88680001,
		0xC78, 0x87690001,
		0xC78, 0x866A0001,
		0xC78, 0x856B0001,
		0xC78, 0x846C0001,
	0xB0000000,0x00000000,
	0x88000001,0x00000000,0x40000000,0x00000000,
		0xC78, 0x616D0001,
		0xC78, 0x486E0001,
		0xC78, 0x476F0001,
		0xC78, 0x46700001,
		0xC78, 0x45710001,
		0xC78, 0x44720001,
		0xC78, 0x43730001,
		0xC78, 0x42740001,
		0xC78, 0x41750001,
		0xC78, 0x40760001,
		0xC78, 0x40770001,
		0xC78, 0x40780001,
		0xC78, 0x40790001,
		0xC78, 0x407A0001,
		0xC78, 0x407B0001,
		0xC78, 0x407C0001,
	0x90000001,0x00000000,0x40000000,0x00000000,
		0xC78, 0x466D0001,
		0xC78, 0x456E0001,
		0xC78, 0x446F0001,
		0xC78, 0x43700001,
		0xC78, 0x42710001,
		0xC78, 0x41720001,
		0xC78, 0x40730001,
		0xC78, 0x40740001,
		0xC78, 0x40750001,
		0xC78, 0x40760001,
		0xC78, 0x40770001,
		0xC78, 0x40780001,
		0xC78, 0x40790001,
		0xC78, 0x407A0001,
		0xC78, 0x407B0001,
		0xC78, 0x407C0001,
	0x98000000,0x00000000,0x40000000,0x00000000,
		0xC78, 0x836D0001,
		0xC78, 0x826E0001,
		0xC78, 0x666F0001,
		0xC78, 0x65700001,
		0xC78, 0x64710001,
		0xC78, 0x63720001,
		0xC78, 0x62730001,
		0xC78, 0x61740001,
		0xC78, 0x48750001,
		0xC78, 0x47760001,
		0xC78, 0x46770001,
		0xC78, 0x45780001,
		0xC78, 0x44790001,
		0xC78, 0x437A0001,
		0xC78, 0x427B0001,
		0xC78, 0x417C0001,
	0xA0000000,0x00000000,
		0xC78, 0x676D0001,
		0xC78, 0x666E0001,
		0xC78, 0x656F0001,
		0xC78, 0x64700001,
		0xC78, 0x63710001,
		0xC78, 0x62720001,
		0xC78, 0x61730001,
		0xC78, 0x60740001,
		0xC78, 0x46750001,
		0xC78, 0x45760001,
		0xC78, 0x44770001,
		0xC78, 0x43780001,
		0xC78, 0x42790001,
		0xC78, 0x417A0001,
		0xC78, 0x407B0001,
		0xC78, 0x407C0001,
	0xB0000000,0x00000000,
		0xC78, 0x407D0001,
		0xC78, 0x407E0001,
		0xC78, 0x407F0001,
		0xC50, 0x69553422,
		0xC50, 0x69553420,

};

void
ODM_ReadAndConfig_MP_8188E_AGC_TAB(
 	IN   PDM_ODM_T  pDM_Odm
 	)
{
    u4Byte     i         = 0;
    u1Byte     cCond;
    BOOLEAN bMatched = TRUE, bSkipped = FALSE;
//ask by Luke.Lee
    u4Byte     ArrayLen    = sizeof(Array_MP_8188E_AGC_TAB)/sizeof(u4Byte);
    pu4Byte    Array       = Array_MP_8188E_AGC_TAB;
	
    ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD, ("===> ODM_ReadAndConfig_MP_8188E_AGC_TAB\n"));

	while(( i+1) < ArrayLen)
	{
		u4Byte v1 = Array[i];
		u4Byte v2 = Array[i+1];

		if(v1 & (BIT31|BIT30)) //positive & negative condition
		{
			if(v1 & BIT31) // positive condition
			{
				cCond  = (u1Byte)((v1 & (BIT29|BIT28)) >> 28);
				if(cCond == COND_ENDIF) //end
				{
					bMatched = TRUE;
					bSkipped = FALSE;
				}
				else if(cCond == COND_ELSE) //else
				{
					bMatched = bSkipped?FALSE:TRUE;
				}
				else //if , else if
				{
					if(bSkipped)
						bMatched = FALSE;
					else
					{
						if(CheckPositive(pDM_Odm, v1, v2))
						{
							bMatched = TRUE;
							bSkipped = TRUE;
						}
						else
						{
							bMatched = FALSE;
							bSkipped = FALSE;
						}
					}
				}
			}
			else if(v1 & BIT30){ //negative condition
			//do nothing
			}
		}
		else
		{
			if(bMatched)
			odm_ConfigBB_AGC_8188E(pDM_Odm, v1, bMaskDWord, v2);
		}
	i = i + 2;
	}
}

u4Byte
ODM_GetVersion_MP_8188E_AGC_TAB(void)
{
	   return 55;
}

/******************************************************************************
*                           PHY_REG.TXT
******************************************************************************/

u4Byte Array_MP_8188E_PHY_REG[] = { 
		0x800, 0x80040000,
		0x804, 0x00000003,
		0x808, 0x0000FC00,
		0x80C, 0x0000000A,
		0x810, 0x10001331,
		0x814, 0x020C3D10,
		0x818, 0x02200385,
		0x81C, 0x00000000,
		0x820, 0x01000100,
		0x824, 0x00390204,
		0x828, 0x00000000,
		0x82C, 0x00000000,
		0x830, 0x00000000,
		0x834, 0x00000000,
		0x838, 0x00000000,
		0x83C, 0x00000000,
		0x840, 0x00010000,
		0x844, 0x00000000,
		0x848, 0x00000000,
		0x84C, 0x00000000,
		0x850, 0x00000000,
		0x854, 0x00000000,
		0x858, 0x569A11A9,
		0x85C, 0x01000014,
		0x860, 0x66F60110,
		0x864, 0x061F0649,
		0x868, 0x00000000,
		0x86C, 0x27272700,
	0x88000003,0x00000000,0x40000000,0x00000000,
		0x870, 0x07000300,
	0x98000001,0x00000000,0x40000000,0x00000000,
		0x870, 0x07000300,
	0x90000003,0x00000000,0x40000000,0x00000000,
		0x870, 0x07000300,
	0x90000001,0x00000000,0x40000000,0x00000000,
		0x870, 0x07000300,
	0xA0000000,0x00000000,
		0x870, 0x07000760,
	0xB0000000,0x00000000,
		0x874, 0x25004000,
		0x878, 0x00000808,
		0x87C, 0x00000000,
		0x880, 0xB0000C1C,
		0x884, 0x00000001,
		0x888, 0x00000000,
		0x88C, 0xCCC000C0,
		0x890, 0x00000800,
		0x894, 0xFFFFFFFE,
		0x898, 0x40302010,
		0x89C, 0x00706050,
		0x900, 0x00000000,
		0x904, 0x00000023,
		0x908, 0x00000000,
		0x90C, 0x81121111,
		0x910, 0x00000002,
		0x914, 0x00000201,
		0xA00, 0x00D047C8,
		0xA04, 0x80FF800C,
		0xA08, 0x8C838300,
		0xA0C, 0x2E7F120F,
		0xA10, 0x9500BB78,
		0xA14, 0x1114D028,
		0xA18, 0x00881117,
		0xA1C, 0x89140F00,
	0x88000003,0x00000000,0x40000000,0x00000000,
		0xA20, 0x13130000,
		0xA24, 0x060A0D10,
		0xA28, 0x00000103,
	0x90000003,0x00000000,0x40000000,0x00000000,
		0xA20, 0x13130000,
		0xA24, 0x060A0D10,
		0xA28, 0x00000103,
	0xA0000000,0x00000000,
		0xA20, 0x1A1B0000,
		0xA24, 0x090E1317,
		0xA28, 0x00000204,
	0xB0000000,0x00000000,
		0xA2C, 0x00D30000,
		0xA70, 0x101FBF00,
		0xA74, 0x00000007,
		0xA78, 0x00000900,
		0xA7C, 0x225B0606,
		0xA80, 0x218075B1,
	0x88000003,0x00000000,0x40000000,0x00000000,
		0xB2C, 0x00000000,
	0x98000001,0x00000000,0x40000000,0x00000000,
		0xB2C, 0x00000000,
	0x90000003,0x00000000,0x40000000,0x00000000,
		0xB2C, 0x00000000,
	0x90000001,0x00000000,0x40000000,0x00000000,
		0xB2C, 0x00000000,
	0xA0000000,0x00000000,
		0xB2C, 0x80000000,
	0xB0000000,0x00000000,
		0xC00, 0x48071D40,
		0xC04, 0x03A05611,
		0xC08, 0x000000E4,
		0xC0C, 0x6C6C6C6C,
		0xC10, 0x08800000,
		0xC14, 0x40000100,
		0xC18, 0x08800000,
		0xC1C, 0x40000100,
		0xC20, 0x00000000,
		0xC24, 0x00000000,
		0xC28, 0x00000000,
		0xC2C, 0x00000000,
		0xC30, 0x69E9AC47,
		0xC34, 0x469652AF,
		0xC38, 0x49795994,
		0xC3C, 0x0A97971C,
		0xC40, 0x1F7C403F,
		0xC44, 0x000100B7,
		0xC48, 0xEC020107,
		0xC4C, 0x007F037F,
		0xC50, 0x69553420,
		0xC54, 0x43BC0094,
	0x88000003,0x00000000,0x40000000,0x00000000,
		0xC58, 0x00013159,
	0x98000001,0x00000000,0x40000000,0x00000000,
		0xC58, 0x00013159,
	0x98000400,0x00000000,0x40000000,0x00000000,
		0xC58, 0x00013159,
	0x98000000,0x00000000,0x40000000,0x00000000,
		0xC58, 0x00013159,
	0xA0000000,0x00000000,
		0xC58, 0x00013169,
	0xB0000000,0x00000000,
		0xC5C, 0x00250492,
		0xC60, 0x00000000,
		0xC64, 0x7112848B,
		0xC68, 0x47C00BFF,
		0xC6C, 0x00000036,
		0xC70, 0x2C7F000D,
	0x88000003,0x00000000,0x40000000,0x00000000,
		0xC74, 0x028610DB,
	0x98000001,0x00000000,0x40000000,0x00000000,
		0xC74, 0x028610DB,
	0x98000400,0x00000000,0x40000000,0x00000000,
		0xC74, 0x028610DB,
	0x98000000,0x00000000,0x40000000,0x00000000,
		0xC74, 0x028610DB,
	0xA0000000,0x00000000,
		0xC74, 0x020610DB,
	0xB0000000,0x00000000,
		0xC78, 0x0000001F,
		0xC7C, 0x00B91612,
	0x88000003,0x00000000,0x40000000,0x00000000,
		0xC80, 0x2D4000B5,
	0x90000003,0x00000000,0x40000000,0x00000000,
		0xC80, 0x2D4000B5,
	0xA0000000,0x00000000,
		0xC80, 0x390000E4,
	0xB0000000,0x00000000,
		0xC84, 0x21F60000,
		0xC88, 0x40000100,
		0xC8C, 0x20200000,
		0xC90, 0x00091521,
		0xC94, 0x00000000,
		0xC98, 0x00121820,
		0xC9C, 0x00007F7F,
		0xCA0, 0x00000000,
		0xCA4, 0x000300A0,
	0x88000003,0x00000000,0x40000000,0x00000000,
		0xCA8, 0xFFFF0000,
	0x98000001,0x00000000,0x40000000,0x00000000,
		0xCA8, 0xFFFF0000,
	0x98000400,0x00000000,0x40000000,0x00000000,
		0xCA8, 0xFFFF0000,
	0x98000000,0x00000000,0x40000000,0x00000000,
		0xCA8, 0xFFFF0000,
	0xA0000000,0x00000000,
		0xCA8, 0x00000000,
	0xB0000000,0x00000000,
		0xCAC, 0x00000000,
		0xCB0, 0x00000000,
		0xCB4, 0x00000000,
		0xCB8, 0x00000000,
		0xCBC, 0x28000000,
		0xCC0, 0x00000000,
		0xCC4, 0x00000000,
		0xCC8, 0x00000000,
		0xCCC, 0x00000000,
		0xCD0, 0x00000000,
		0xCD4, 0x00000000,
		0xCD8, 0x64B22427,
		0xCDC, 0x00766932,
		0xCE0, 0x00222222,
		0xCE4, 0x00000000,
		0xCE8, 0x37644302,
		0xCEC, 0x2F97D40C,
		0xD00, 0x00000740,
		0xD04, 0x00020401,
		0xD08, 0x0000907F,
		0xD0C, 0x20010201,
		0xD10, 0xA0633333,
		0xD14, 0x3333BC43,
		0xD18, 0x7A8F5B6F,
		0xD2C, 0xCC979975,
		0xD30, 0x00000000,
		0xD34, 0x80608000,
		0xD38, 0x00000000,
		0xD3C, 0x00127353,
		0xD40, 0x00000000,
		0xD44, 0x00000000,
		0xD48, 0x00000000,
		0xD4C, 0x00000000,
		0xD50, 0x6437140A,
		0xD54, 0x00000000,
		0xD58, 0x00000282,
		0xD5C, 0x30032064,
		0xD60, 0x4653DE68,
		0xD64, 0x04518A3C,
		0xD68, 0x00002101,
		0xD6C, 0x2A201C16,
		0xD70, 0x1812362E,
		0xD74, 0x322C2220,
		0xD78, 0x000E3C24,
		0xE00, 0x2D2D2D2D,
		0xE04, 0x2D2D2D2D,
		0xE08, 0x0390272D,
		0xE10, 0x2D2D2D2D,
		0xE14, 0x2D2D2D2D,
		0xE18, 0x2D2D2D2D,
		0xE1C, 0x2D2D2D2D,
		0xE28, 0x00000000,
		0xE30, 0x1000DC1F,
		0xE34, 0x10008C1F,
		0xE38, 0x02140102,
		0xE3C, 0x681604C2,
		0xE40, 0x01007C00,
		0xE44, 0x01004800,
		0xE48, 0xFB000000,
		0xE4C, 0x000028D1,
		0xE50, 0x1000DC1F,
		0xE54, 0x10008C1F,
		0xE58, 0x02140102,
		0xE5C, 0x28160D05,
		0xE60, 0x00000008,
		0xE68, 0x001B25A4,
		0xE6C, 0x00C00014,
		0xE70, 0x00C00014,
		0xE74, 0x01000014,
		0xE78, 0x01000014,
		0xE7C, 0x01000014,
		0xE80, 0x01000014,
		0xE84, 0x00C00014,
		0xE88, 0x01000014,
		0xE8C, 0x00C00014,
		0xED0, 0x00C00014,
		0xED4, 0x00C00014,
		0xED8, 0x00C00014,
		0xEDC, 0x00000014,
		0xEE0, 0x00000014,
	0x88000003,0x00000000,0x40000000,0x00000000,
		0xEE8, 0x32555448,
	0x98000001,0x00000000,0x40000000,0x00000000,
		0xEE8, 0x32555448,
	0x90000003,0x00000000,0x40000000,0x00000000,
		0xEE8, 0x32555448,
	0x90000001,0x00000000,0x40000000,0x00000000,
		0xEE8, 0x32555448,
	0xA0000000,0x00000000,
		0xEE8, 0x21555448,
	0xB0000000,0x00000000,
		0xEEC, 0x01C00014,
		0xF14, 0x00000003,
		0xF4C, 0x00000000,
		0xF00, 0x00000300,

};

void
ODM_ReadAndConfig_MP_8188E_PHY_REG(
 	IN   PDM_ODM_T  pDM_Odm
 	)
{
    u4Byte     i         = 0;
    u1Byte     cCond;
    BOOLEAN bMatched = TRUE, bSkipped = FALSE;
//ask by Luke.Lee
    u4Byte     ArrayLen    = sizeof(Array_MP_8188E_PHY_REG)/sizeof(u4Byte);
    pu4Byte    Array       = Array_MP_8188E_PHY_REG;
	
    ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD, ("===> ODM_ReadAndConfig_MP_8188E_PHY_REG\n"));

	while(( i+1) < ArrayLen)
	{
		u4Byte v1 = Array[i];
		u4Byte v2 = Array[i+1];

		if(v1 & (BIT31|BIT30)) //positive & negative condition
		{
			if(v1 & BIT31) // positive condition
			{
				cCond  = (u1Byte)((v1 & (BIT29|BIT28)) >> 28);
				if(cCond == COND_ENDIF) //end
				{
					bMatched = TRUE;
					bSkipped = FALSE;
				}
				else if(cCond == COND_ELSE) //else
				{
					bMatched = bSkipped?FALSE:TRUE;
				}
				else //if , else if
				{
					if(bSkipped)
						bMatched = FALSE;
					else
					{
						if(CheckPositive(pDM_Odm, v1, v2))
						{
							bMatched = TRUE;
							bSkipped = TRUE;
						}
						else
						{
							bMatched = FALSE;
							bSkipped = FALSE;
						}
					}
				}
			}
			else if(v1 & BIT30){ //negative condition
			//do nothing
			}
		}
		else
		{
			if(bMatched)
			odm_ConfigBB_PHY_8188E(pDM_Odm, v1, bMaskDWord, v2);
		}
	i = i + 2;
	}
}

u4Byte
ODM_GetVersion_MP_8188E_PHY_REG(void)
{
	   return 55;
}

/******************************************************************************
*                           PHY_REG_PG.TXT
******************************************************************************/

u4Byte Array_MP_8188E_PHY_REG_PG[] = { 
	0, 0, 0, 0x00000e08, 0x0000ff00, 0x00003800,
	0, 0, 0, 0x0000086c, 0xffffff00, 0x32343600,
	0, 0, 0, 0x00000e00, 0xffffffff, 0x40424446,
	0, 0, 0, 0x00000e04, 0xffffffff, 0x28323638,
	0, 0, 0, 0x00000e10, 0xffffffff, 0x38404244,
	0, 0, 0, 0x00000e14, 0xffffffff, 0x26303436
};

void
ODM_ReadAndConfig_MP_8188E_PHY_REG_PG(
 	IN   PDM_ODM_T  pDM_Odm
 	)
{
    u4Byte     i         = 0;
    u4Byte     ArrayLen    = sizeof(Array_MP_8188E_PHY_REG_PG)/sizeof(u4Byte);
    pu4Byte    Array       = Array_MP_8188E_PHY_REG_PG;

    ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD, ("===> ODM_ReadAndConfig_MP_8188E_PHY_REG_PG\n"));

	pDM_Odm->PhyRegPgVersion = 1;
	pDM_Odm->PhyRegPgValueType = PHY_REG_PG_EXACT_VALUE;

	for (i = 0; i < ArrayLen; i += 6 )
	{
	    u4Byte v1 = Array[i];
	    u4Byte v2 = Array[i+1];
	    u4Byte v3 = Array[i+2];
	    u4Byte v4 = Array[i+3];
	    u4Byte v5 = Array[i+4];
	    u4Byte v6 = Array[i+5];

	    odm_ConfigBB_PHY_REG_PG_8188E(pDM_Odm, v1, v2, v3, v4, v5, v6);
	}
}



#endif // end of HWIMG_SUPPORT

