/****************************************************************************

    MODULE:         SWD_GUID.HPP
    Tab Settings:   5 9
    Copyright 1995, 1996, Microsoft Corporation,    All Rights Reserved.

    PURPOSE:        CLSIDs and IIDs defined for DirectInputForce

    FUNCTIONS:

    Author(s):  Name:
    ----------  ----------------

    Revision History:
    -----------------
    Version     Date        Author  Comments
    -------     ------      -----   -------------------------------------------
    1.0         06-Feb-97   MEA     original, Based on SWForce
                23-Feb-97   MEA     Modified for DirectInput FF Device Driver
    1.1         14-Apr-97   MEA     Added GUID_RTCSpring

****************************************************************************/
#ifndef _SWD_GUID_SEEN
#define _SWD_GUID_SEEN

#ifdef INITGUIDS
#include <initguid.h>
#endif //INITGUIDS


/*
 * GUIDs
 *
 */


//
// --- VFX Class ID
//
DEFINE_GUID ( CLSID_VFX, /* 04ace0a7-1fa8-11d0-aa22-00a0c911f471 */
              0x04ace0a7,
              0x1fa8,
              0x11d0,
              0xaa, 0x22, 0x00, 0xa0, 0xc9, 0x11, 0xf4, 0x71 );

//
// --- VFX Interface
//
DEFINE_GUID ( IID_IVFX, /* 04ace0a6-1fa8-11d0-aa22-00a0c911f471 */
              0x04ace0a6,
              0x1fa8,
              0x11d0,
              0xaa, 0x22, 0x00, 0xa0, 0xc9, 0x11, 0xf4, 0x71 );

//
// --- Effect GUIDs
//
DEFINE_GUID ( GUID_Wall, /* e84cd1a1-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1a1,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );

#if 0
// Built in ROM Effects
DEFINE_GUID ( GUID_RandomNoise, /* e84cd1a3-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1a3,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );

DEFINE_GUID ( GUID_AircraftCarrierTakeOff, /* e84cd1a4-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1a4,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );

DEFINE_GUID ( GUID_BasketballDribble, /* e84cd1a5-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1a5,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );
DEFINE_GUID ( GUID_CarEngineIdle, /* e84cd1a6-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1a6,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );
DEFINE_GUID ( GUID_ChainsawIdle, /* e84cd1a7-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1a7,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );
DEFINE_GUID ( GUID_ChainsawInAction, /* e84cd1a8-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1a8,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );
DEFINE_GUID ( GUID_DieselEngineIdle, /* e84cd1a9-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1a9,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );
DEFINE_GUID ( GUID_Jump, /* e84cd1aa-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1aa,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );
DEFINE_GUID ( GUID_Land, /* e84cd1ab-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1ab,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );
DEFINE_GUID ( GUID_MachineGun, /* e84cd1ac-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1ac,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );
DEFINE_GUID ( GUID_Punched, /* e84cd1ad-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1ad,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );
DEFINE_GUID ( GUID_RocketLaunch, /* e84cd1ae-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1ae,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );
DEFINE_GUID ( GUID_SecretDoor, /* e84cd1af-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1af,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );
DEFINE_GUID ( GUID_SwitchClick, /* e84cd1b0-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1b0,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );

DEFINE_GUID ( GUID_WindGust, /* e84cd1b1-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1b1,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );

DEFINE_GUID ( GUID_WindShear, /* e84cd1b2-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1b2,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );

DEFINE_GUID ( GUID_Pistol, /* e84cd1b3-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1b3,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );

DEFINE_GUID ( GUID_Shotgun, /* e84cd1b4-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1b4,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );

DEFINE_GUID ( GUID_Laser1, /* e84cd1b5-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1b5,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );

DEFINE_GUID ( GUID_Laser2, /* e84cd1b6-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1b6,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );

DEFINE_GUID ( GUID_Laser3, /* e84cd1b7-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1b7,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );

DEFINE_GUID ( GUID_Laser4, /* e84cd1b8-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1b8,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );

DEFINE_GUID ( GUID_Laser5, /* e84cd1b9-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1b9,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );

DEFINE_GUID ( GUID_Laser6, /* e84cd1ba-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1ba,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );

DEFINE_GUID ( GUID_OutOfAmmo, /* e84cd1bb-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1bb,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );

DEFINE_GUID ( GUID_LightningGun, /* e84cd1bc-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1bc,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );

DEFINE_GUID ( GUID_Missile, /* e84cd1bd-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1bd,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );

DEFINE_GUID ( GUID_GatlingGun, /* e84cd1be-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1be,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );

DEFINE_GUID ( GUID_ShortPlasma, /* e84cd1bf-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1bf,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );

DEFINE_GUID ( GUID_PlasmaCannon1, /* e84cd1c0-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1c0,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );

DEFINE_GUID ( GUID_PlasmaCannon2, /* e84cd1c1-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1c1,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );

DEFINE_GUID ( GUID_Cannon, /* e84cd1c2-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1c2,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );
#endif

DEFINE_GUID ( GUID_RawForce, /* e84cd1c6-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1c6,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );

DEFINE_GUID ( GUID_VFXEffect, /* e84cd1c7-81fa-11d0-94ab-0080c74c7e95 */
              0xe84cd1c7,
              0x81fa,
              0x11d0,
              0x94, 0xab, 0x00, 0x80, 0xc7, 0x4c, 0x7e, 0x95
            );

#endif //_SWD_GUID_SEEN

