/**
* SBGuardian v2.0
* Copyright (c) 2011 Fabian Fischer
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
**/

#include "hl1.h"
#include "games.h"
#include "ServerPlugin.h"
#include "../version.h"
#include "../SBGuardian.h"
#undef open
#undef close
#include "../DLL.h"


typedef void (WINAPI2 *GEF) (enginefuncs_t *pengfuncsFromEngine, globalvars_t *pGlobals);
typedef int (*GETENTITYAPI) (DLL_FUNCTIONS *pFunctionTable, int interfaceVersion);
typedef int (*GETENTITYAPI2) (DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion);
typedef int (*GETNEWDLLFUNCTIONS) (NEW_DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion);

int NewInterfaceVersion = 1;
int InterfaceVersion = 140;

enginefuncs_t g_EngFuncs;
NEW_DLL_FUNCTIONS g_NewDllFuncs;
DLL_FUNCTIONS g_FunctionTable;
CDLL Dll;
globalvars_t *g_Globals = NULL;
globalvars_t *gpGlobals = NULL;
enginefuncs_t g_engfuncs;

void *addrFuncPtrsToDll = NULL;
void *addrNewDllFuncs = NULL;
void *addrEntityAPI2 = NULL;
void *addrEntityAPI = NULL;

void LoadFuncPtrsToDll(void *engfuncs)
{
	enginefuncs_t *engfuncs2 = (enginefuncs_t *) engfuncs;
	GEF gef = (GEF) addrFuncPtrsToDll;
	if( engfuncs2 == NULL ) gef(&g_EngFuncs, g_Globals);
	else gef(engfuncs2, g_Globals);
}

void LoadNewDLLFuncs(void *ndf)
{
	NEW_DLL_FUNCTIONS *ndf2 = (NEW_DLL_FUNCTIONS *) ndf;
	GETNEWDLLFUNCTIONS gndf = (GETNEWDLLFUNCTIONS) addrNewDllFuncs;
	if( ndf2 == NULL ) gndf(&g_NewDllFuncs ,&NewInterfaceVersion);
	else if(!gndf(ndf2, &NewInterfaceVersion))
	{
		printf("Using old Version. New Version: %d\n", NewInterfaceVersion);
	}
}

void LoadEntityAPI2(void *pFuncTable)
{
	DLL_FUNCTIONS *pFuncTable2 = (DLL_FUNCTIONS *) pFuncTable;
	GETENTITYAPI2 gea2 = (GETENTITYAPI2) addrEntityAPI2;
	if( pFuncTable2 == NULL ) gea2(&g_FunctionTable, &InterfaceVersion);
	else if(!gea2(pFuncTable2, &InterfaceVersion))
	{
		printf("Using old Version. New Version: %d\n", InterfaceVersion);
	}
}

void LoadEntityAPI(void *pFuncTable)
{
	DLL_FUNCTIONS *pFuncTable2 = (DLL_FUNCTIONS *) pFuncTable;
	GETENTITYAPI gea = (GETENTITYAPI) addrEntityAPI;
	if( pFuncTable2 == NULL ) gea(&g_FunctionTable, InterfaceVersion);
	else if(!gea(pFuncTable2, InterfaceVersion))
	{
		printf("Using old Version. New Version: %d\n", InterfaceVersion);
	}
}

// sets dest's ptrs to source's ptrs if source-ptr != NULL
void setPtr(void *destination, void *source, size_t size) {
	int **src = (int **) source;
	int **dest = (int **) destination;
	for( size_t i = 0; i < size/4; i++ ) {
		if( src[i] != NULL )
			dest[i] = src[i];
	}
}

#define ENGINE_INTERFACE_VERSION 138

enginefuncs_t EngFuncs =
{
	NULL,						// pfnPrecacheModel()
	NULL,						// pfnPrecacheSound()
	NULL,						// pfnSetModel()
	NULL,						// pfnModelIndex()
	NULL,						// pfnModelFrames()

	NULL,						// pfnSetSize()
	NULL,						// pfnChangeLevel()
	NULL,						// pfnGetSpawnParms()
	NULL,						// pfnSaveSpawnParms()

	NULL,						// pfnVecToYaw()
	NULL,						// pfnVecToAngles()
	NULL,						// pfnMoveToOrigin()
	NULL,						// pfnChangeYaw()
	NULL,						// pfnChangePitch()

	NULL,						// pfnFindEntityByString()
	NULL,						// pfnGetEntityIllum()
	NULL,						// pfnFindEntityInSphere()
	NULL,						// pfnFindClientInPVS()
	NULL,						// pfnEntitiesInPVS()

	NULL,						// pfnMakeVectors()
	NULL,						// pfnAngleVectors()

	NULL,						// pfnCreateEntity()
	NULL,						// pfnRemoveEntity()
	NULL,						// pfnCreateNamedEntity()

	NULL,						// pfnMakeStatic()
	NULL,						// pfnEntIsOnFloor()
	NULL,						// pfnDropToFloor()

	NULL,						// pfnWalkMove()
	NULL,						// pfnSetOrigin()
	NULL,						// pfnEmitSound()
	NULL,						// pfnEmitAmbientSound()
	NULL,						// pfnTraceLine()
	NULL,						// pfnTraceToss()
	NULL,						// pfnTraceMonsterHull()
	NULL,						// pfnTraceHull()
	NULL,						// pfnTraceModel()
	NULL,						// pfnTraceTexture()
	NULL,						// pfnTraceSphere()
	NULL,						// pfnGetAimVector()

	NULL,						// pfnServerCommand()
	NULL,						// pfnServerExecute()
	NULL,						// pfnClientCommand()

	NULL,						// pfnParticleEffect()
	NULL,						// pfnLightStyle()
	NULL,						// pfnDecalIndex()
	NULL,						// pfnPointContents()

	CServerPlugin::MessageBegin,
	CServerPlugin::MessageEnd,
	CServerPlugin::WriteByte,
	CServerPlugin::WriteChar,						// pfnWriteChar()
	CServerPlugin::WriteShort,
	CServerPlugin::WriteLong,						// pfnWriteLong()
	CServerPlugin::WriteAngle,						// pfnWriteAngle()
	CServerPlugin::WriteCoord,						// pfnWriteCoord()
	CServerPlugin::WriteString,						// pfnWriteString()
	CServerPlugin::WriteEntity,						// pfnWriteEntity()

	NULL,						// pfnCVarRegister()
	NULL,						// pfnCVarGetFloat()
	NULL,						// pfnCVarGetString()
	NULL,						// pfnCVarSetFloat()
	NULL,						// pfnCVarSetString()

	NULL,						// pfnAlertMessage()
	NULL,						// pfnEngineFprintf()

	NULL,						// pfnPvAllocEntPrivateData()
	NULL,						// pfnPvEntPrivateData()
	NULL,						// pfnFreeEntPrivateData()

	NULL,						// pfnSzFromIndex()
	NULL,						// pfnAllocString()

	NULL, 						// pfnGetVarsOfEnt()
	NULL,						// pfnPEntityOfEntOffset()
	NULL,						// pfnEntOffsetOfPEntity()
	NULL, // ah_IndexofEdict,	// pfnIndexOfEdict()
	NULL,						// pfnPEntityOfEntIndex()
	NULL,						// pfnFindEntityByVars()
	NULL,						// pfnGetModelPtr()
	CServerPlugin::RegUserMsg,			// pfnRegUserMsg()

	NULL,						// pfnAnimationAutomove()
	NULL,						// pfnGetBonePosition()

	NULL,						// pfnFunctionFromName()
	NULL,						// pfnNameForFunction()

	NULL,						// pfnClientPrintf()
	NULL,						// pfnServerPrint()

	NULL,						// pfnCmd_Args()
	NULL,						// pfnCmd_Argv()
	NULL,						// pfnCmd_Argc()

	NULL,						// pfnGetAttachment()

	NULL,						// pfnCRC32_Init()
	NULL,						// pfnCRC32_ProcessBuffer()
	NULL,						// pfnCRC32_ProcessByte()
	NULL,						// pfnCRC32_Final()

	NULL,						// pfnRandomLong()
	NULL,						// pfnRandomFloat()

	NULL,						// pfnSetView()
	NULL,						// pfnTime()
	NULL,						// pfnCrosshairAngle()

	NULL,						// pfnLoadFileForMe()
	NULL,						// pfnFreeFile()

	NULL,						// pfnEndSection()
	NULL,						// pfnCompareFileTime()
	NULL,						// pfnGetGameDir()
	NULL,						// pfnCvar_RegisterVariable()
	NULL,						// pfnFadeClientVolume()
	NULL,						// pfnSetClientMaxspeed()
	NULL,						// pfnCreateFakeClient()
	NULL,						// pfnRunPlayerMove()
	NULL,						// pfnNumberOfEntities()

	NULL,						// pfnGetInfoKeyBuffer()
	NULL,						// pfnInfoKeyValue()
	NULL,						// pfnSetKeyValue()
	NULL,						// pfnSetClientKeyValue()

	NULL,						// pfnIsMapValid()
	NULL,						// pfnStaticDecal()
	NULL,						// pfnPrecacheGeneric()
	NULL, 						// pfnGetPlayerUserId()
	NULL,						// pfnBuildSoundMsg()
	NULL,						// pfnIsDedicatedServer()
	NULL,						// pfnCVarGetPointer()
	NULL,						// pfnGetPlayerWONId()

	NULL,						// pfnInfo_RemoveKey()
	NULL,						// pfnGetPhysicsKeyValue()
	NULL,						// pfnSetPhysicsKeyValue()
	NULL,						// pfnGetPhysicsInfoString()
	NULL,						// pfnPrecacheEvent()
	NULL,						// pfnPlaybackEvent()

	NULL,						// pfnSetFatPVS()
	NULL,						// pfnSetFatPAS()

	NULL,						// pfnCheckVisibility()

	NULL,						// pfnDeltaSetField()
	NULL,						// pfnDeltaUnsetField()
	NULL,						// pfnDeltaAddEncoder()
	NULL,						// pfnGetCurrentPlayer()
	NULL,						// pfnCanSkipPlayer()
	NULL,						// pfnDeltaFindField()
	NULL,						// pfnDeltaSetFieldByIndex()
	NULL,						// pfnDeltaUnsetFieldByIndex()

	NULL,						// pfnSetGroupMask()

	NULL,						// pfnCreateInstancedBaseline()
	NULL,						// pfnCvar_DirectSet()

	NULL,						// pfnForceUnmodified()

	NULL,						// pfnGetPlayerStats()

	NULL,						// pfnAddServerCommand()

	// Added in SDK 2.2:
	NULL,						// pfnVoice_GetClientListening()
	NULL,						// pfnVoice_SetClientListening()

	// Added for HL 1109 (no SDK update):
	NULL,						// pfnGetPlayerAuthId()

	// Added 2003-11-10 (no SDK update):
	NULL,						// pfnSequenceGet()
	NULL,						// pfnSequencePickSen		mutil_funcs_t *pMetaUtilFuncs)
	NULL,						// pfnGetFileSize()
	NULL,						// pfnGetApproxWavePlayLen()
	NULL,						// pfnIsCareerMatch()
	NULL,						// pfnGetLocalizedStringLength()
	NULL,						// pfnRegisterTutorMessageShown()
	NULL,						// pfnGetTimesTutorMessageShown()
	NULL,						// pfnProcessTutorMessageDecayBuffer()
	NULL,						// pfnConstructTutorMessageDecayBuffer()
	NULL,						// pfnResetTutorMessageDecayData()

	// Added Added 2005-08-11 (no SDK update)
	NULL,						// pfnQueryClientCvarValue()
	// Added Added 2005-11-22 (no SDK update)
	NULL,						// pfnQueryClientCvarValue2()
};

DLL_FUNCTIONS FunctionTable =
{
	NULL,					// pfnGameInit
	NULL,					// pfnSpawn
	NULL,					// pfnThink
	NULL,					// pfnUse
	NULL,					// pfnTouch
	NULL,					// pfnBlocked
	NULL,					// pfnKeyValue
	NULL,					// pfnSave
	NULL,					// pfnRestore
	NULL,					// pfnSetAbsBox

	NULL,					// pfnSaveWriteFields
	NULL,					// pfnSaveReadFields

	NULL,					// pfnSaveGlobalState
	NULL,					// pfnRestoreGlobalState
	NULL,					// pfnResetGlobalState

	NULL, // ah_ClientConnect2,		// pfnClientConnect
	CServerPlugin::ClientDisconnect,	// pfnClientDisconnect
	NULL,					// pfnClientKill
	CServerPlugin::ClientPutInServer,		// pfnClientPutInServer
	CServerPlugin::ClientCommand,		// pfnClientCommand
	NULL, // CServerPlugin::ClientSettingsChanged,					// pfnClientUserInfoChanged
	CServerPlugin::ServerActivate,	// pfnServerActivate
	CServerPlugin::ServerDeactivate,					// pfnServerDeactivate

	NULL,					// pfnPlayerPreThink
	NULL,					// pfnPlayerPostThink

	CServerPlugin::GameFrame,			// pfnStartFrame
	NULL,					// pfnParmsNewLevel
	NULL,					// pfnParmsChangeLevel

	NULL,					// pfnGetGameDescription
	NULL,					// pfnPlayerCustomization

	NULL,					// pfnSpectatorConnect
	NULL,					// pfnSpectatorDisconnect
	NULL,					// pfnSpectatorThink

	NULL,					// pfnSys_Error
	NULL, // CServerPlugin::onPM_Move, 			// pfnPM_Move
	NULL,					// pfnPM_Init
	NULL,					// pfnPM_FindTextureType

	NULL,					// pfnSetupVisibility
	NULL,					// pfnUpdateClientData
	CServerPlugin::CheckTransmit,		// pfnAddToFullPack
	NULL,					// pfnCreateBaseline
	NULL,					// pfnRegisterEncoders
	NULL,					// pfnGetWeaponData
	NULL,					// pfnCmdStart
	NULL,					// pfnCmdEnd
	NULL,					// pfnConnectionlessPacket
	NULL,					// pfnGetHullBounds
	NULL,					// pfnCreateInstancedBaselines
	NULL,					// pfnInconsistentFile
	NULL,					// pfnAllowLagCompensation
};

NEW_DLL_FUNCTIONS NewDllFuncs =
{
	NULL,			// void (*pfnOnFreeEntPrivateData)(edict_t *pEnt);
	NULL,			// void	(*pfnGameShutdown)(void);
	NULL,			// int (*pfnShouldCollide)( edict_t *pentTouched, edict_t *pentOther );
	NULL,			// void	(*pfnCvarValue)( const edict_t *pEnt, const char *value );
	CServerPlugin::OnQueryCvarValueFinished	// void (*pfnCvarValue2)( const edict_t *pEnt, int requestID, const char *cvarName, const char *value );
};

// MetaMod
extern "C" DLLEXP int GetEngineFunctions(enginefuncs_t *pengfuncsFromEngine, int *interfaceVersion)
{
	if(!pengfuncsFromEngine) {
		printf("GetEngineFunctions called with null pengfuncsFromEngine\n");
		return(FALSE);
	}
	else if(*interfaceVersion != ENGINE_INTERFACE_VERSION) {
		printf("GetEngineFunctions version mismatch; requested=%d ours=%d\n", *interfaceVersion, ENGINE_INTERFACE_VERSION);
		*interfaceVersion = ENGINE_INTERFACE_VERSION;
		return(FALSE);
	}

	memcpy(pengfuncsFromEngine, &EngFuncs, sizeof(enginefuncs_t));
	return(TRUE);
}

extern "C" DLLEXP int GetEntityAPI2(DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion)
{
	if(!pFunctionTable) {
		printf("GetEntityAPI2 called with null pFunctionTable\n");
		return(FALSE);
	}
	else if(*interfaceVersion != INTERFACE_VERSION) {
		printf("GetEntityAPI2 version mismatch; requested=%d ours=%d\n", *interfaceVersion, INTERFACE_VERSION);
		*interfaceVersion = INTERFACE_VERSION;
		return(FALSE);
	}

#ifndef STANDALONE
	memcpy(pFunctionTable, &FunctionTable, sizeof(DLL_FUNCTIONS));
#else
	memcpy(pFunctionTable, &g_FunctionTable, sizeof(DLL_FUNCTIONS));
	setPtr(pFunctionTable, &FunctionTable, sizeof(DLL_FUNCTIONS));
#endif
	return(TRUE);
}


extern "C" DLLEXP int GetEntityAPI( DLL_FUNCTIONS *pFunctionTable, int interfaceVersion )
{
	if ( !pFunctionTable || interfaceVersion != INTERFACE_VERSION )
	{
		return FALSE;
	}

#ifndef STANDALONE
	memcpy(pFunctionTable, &FunctionTable, sizeof(DLL_FUNCTIONS));
#else
	memcpy(pFunctionTable, &g_FunctionTable, sizeof(DLL_FUNCTIONS));
	setPtr(pFunctionTable, &FunctionTable, sizeof(DLL_FUNCTIONS));
#endif
	return TRUE;
}

extern "C" DLLEXP int GetNewDLLFunctions( NEW_DLL_FUNCTIONS *pNewFunctionTable, int *interfaceVersion )
{

#ifndef STANDALONE
	memcpy(pNewFunctionTable, &NewDllFuncs, sizeof(NEW_DLL_FUNCTIONS));
#else
	memcpy(pNewFunctionTable, &g_NewDllFuncs, sizeof(NEW_DLL_FUNCTIONS));
	setPtr(pNewFunctionTable, &NewDllFuncs, sizeof(NEW_DLL_FUNCTIONS));
#endif
	return 1;
}

extern "C" DLLEXP void WINAPI GiveFnptrsToDll( enginefuncs_t* pengfuncsFromEngine, globalvars_t *pGlobals )
{
	memcpy(&g_EngFuncs, pengfuncsFromEngine, sizeof(enginefuncs_t));
	memcpy(&g_engfuncs, pengfuncsFromEngine, sizeof(enginefuncs_t));
	g_Globals = pGlobals;
	gpGlobals = pGlobals;
	
#ifdef STANDALONE
	setPtr(pengfuncsFromEngine, &EngFuncs, sizeof(enginefuncs_t));

	char gamedir[512];
	char dll[512+256];
	g_EngFuncs.pfnGetGameDir(gamedir);
	int i;
	for(i = 0; i < sizeof(dllnames)/(sizeof(dllnames[1])*sizeof(char)); i++)
	{
		strcpy(dll, gamedir);
		strcat(dll, "/dlls/");
		strcat(dll, dllnames[i]);

#ifdef WIN32
		strcat(dll, ".dll");
#else

#ifdef __amd64__
		strcat(dll, "_amd64.so");
#else
		strcat(dll, "_i386.so");
#endif

#endif

		Dll.open(dll);
		if(Dll.isOpen())
			break;

		memset(dll, 0, sizeof(dll));
		Dll.close();
	}

	if(!(addrFuncPtrsToDll = Dll.sym("GiveFnptrsToDll")))
	{
		// giveFNs(addr);
	}

	if(!(addrNewDllFuncs = Dll.sym("GetNewDLLFunctions")))
	{
		// load_getNewDLLFunctions(addr);
	}

	if(!(addrEntityAPI2 = Dll.sym("GetEntityAPI2" )))
	{
		// load_GetEntityAPI2(addr);
	}

	if(!(addrEntityAPI = Dll.sym("GetEntityAPI")))
	{
		// load_GetEntityAPI(addr);
	}

	LoadFuncPtrsToDll(pengfuncsFromEngine);
	LoadNewDLLFuncs(&g_NewDllFuncs);
	// LoadEntityAPI2(&g_FunctionTable2);
	LoadEntityAPI(&g_FunctionTable);
	// g_CServerPlugin.Init();
	return;
#endif

	// g_CServerPlugin.Init();

}

#ifndef STANDALONE

#include <meta_api.h>

// Must provide at least one of these..
static META_FUNCTIONS gMetaFunctionTable = {
	NULL,			// pfnGetEntityAPI				HL SDK; called before game DLL
	NULL,			// pfnGetEntityAPI_Post			META; called after game DLL
	GetEntityAPI2,	// pfnGetEntityAPI2				HL SDK2; called before game DLL
	NULL, // GetEntityAPI2_Post,	// pfnGetEntityAPI2_Post		META; called after game DLL
	GetNewDLLFunctions,			// pfnGetNewDLLFunctions	HL SDK2; called before game DLL
	NULL,			// pfnGetNewDLLFunctions_Post	META; called after game DLL
	GetEngineFunctions,	// pfnGetEngineFunctions	META; called before HL engine
	NULL,			// pfnGetEngineFunctions_Post	META; called after HL engine
};

// Description of plugin
plugin_info_t Plugin_info = {
	META_INTERFACE_VERSION,	// ifvers
	NAME,	// name
	VERSION,	// version
	__DATE__,	// date
	AUTHOR,	// author
	URL,	// url
	PLUGINTAG,	// logtag, all caps please
	PT_ANYPAUSE,	// (when) loadable
	PT_ANYPAUSE,	// (when) unloadable
};

// Global vars from metamod:
meta_globals_t *gpMetaGlobals;		// metamod globals
gamedll_funcs_t *gpGamedllFuncs;	// gameDLL function tables
mutil_funcs_t *gpMetaUtilFuncs;		// metamod utility functions

extern "C" int Meta_Attach(PLUG_LOADTIME  now,
		META_FUNCTIONS *pFunctionTable, meta_globals_t *pMGlobals,
		gamedll_funcs_t *pGamedllFuncs)
{
	if(!pMGlobals) {
		LOG_ERROR(PLID, "Meta_Attach called with null pMGlobals");
		return(FALSE);
	}
	gpMetaGlobals=pMGlobals;
	if(!pFunctionTable) {
		LOG_ERROR(PLID, "Meta_Attach called with null pFunctionTable");
		return(FALSE);
	}
	memcpy(pFunctionTable, &gMetaFunctionTable, sizeof(META_FUNCTIONS));
	gpGamedllFuncs=pGamedllFuncs;

	// g_CServerPlugin.Init();

	/* InitLate*/
	if(now > PT_STARTUP)
	{
		g_ServerPlugin.init(true);
	}

	return(TRUE);
}

extern "C" int Meta_Detach(PLUG_LOADTIME /* now */,
		PL_UNLOAD_REASON /* reason */)
{
	// g_SBG.unload();
	g_ServerPlugin.unload();
	return(TRUE);
}

extern "C" int Meta_Query(char * /*ifvers */, plugin_info_t **pPlugInfo,
		mutil_funcs_t *pMetaUtilFuncs)
{
	// Give metamod our plugin_info struct
	*pPlugInfo=&Plugin_info;
	// Get metamod utility function table.
	gpMetaUtilFuncs=pMetaUtilFuncs;

	return(TRUE);
}

#endif

#ifdef STANDALONE

#define DLL_PROCESS_ATTACH 1
#define DLL_PROCESS_DETACH 0

#ifdef WIN32
// Required DLL entry point
extern "C" BOOL WINAPI DllMain(
   HINSTANCE hinstDLL,
   DWORD fdwReason,
   LPVOID lpvReserved)
{
	if(fdwReason == DLL_PROCESS_ATTACH)
    {
    }
	else if(fdwReason == DLL_PROCESS_DETACH)
    {
    }
	return TRUE;
}
#endif

#endif



