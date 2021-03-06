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

#include "SBGuardian.h"
#include "Arg.h"
#include "CStr.h"
#include "version.h"
#include "profile.h"

CSBGuardian g_SBG;

bool CSBGuardian::load(IPlayerManager *pPlayerManager, IEngine *pEngine) {
	m_pPlayerManager = pPlayerManager;
	m_pEngine = pEngine;

	m_pWh = new CAntiWallhack();
	m_pFlash = new CAntiFlashhack();
	m_pFov = new CFOVCheck();
	m_pCvar = new CCvarCheck();
	//m_pUpdate = new CUpdateSystem();
	m_pAb = new CAntiAimbot();
	m_pWm = new CWarmodus((IModule **) &m_pWh);
	// m_pSh = new CAntiSpeedhack();

	m_vecModule.push_back(m_pWh);
	m_vecModule.push_back(m_pFlash);
	m_vecModule.push_back(m_pFov);
	m_vecModule.push_back(m_pCvar);
	m_vecModule.push_back(m_pAb);
	m_vecModule.push_back(m_pWm);
	//m_vecModule.push_back(m_pUpdate);
	//m_vecModule.push_back(m_pSh);

	// init all modules
	for(vector<IModule*>::iterator it = m_vecModule.begin(); it < m_vecModule.end(); ++it) {
		(*it)->init(pEngine, pPlayerManager);
	}

	m_pEngine->command("exec sbg.cfg\n");

	m_bLoaded = true;
	return true;
}

void CSBGuardian::unload() {
	m_bLoaded = false;

	// unload all modules
	for(vector<IModule*>::iterator it = m_vecModule.begin(); it < m_vecModule.end(); ++it) {
		delete *it;
	}
	m_vecModule.clear();
	m_pWh = NULL;
	m_pFlash = NULL;
	m_pFov = NULL;
	m_pCvar = NULL;
	m_pAb = NULL;
	m_pWm = NULL;
	m_pUpdate = NULL;
}

void CSBGuardian::onMapChange() {
	// m_pUpdate->onMapChange();
}

void CSBGuardian::onFrame() {
	m_pWh->onFrame();
	m_pCvar->onFrame();
	//m_pUpdate->onFrame();
	m_pAb->onFrame();
	m_pWm->onFrame();
	// m_pSh->onFrame();
	return;

	/* InitLate, loaded after config.cfg got executed */
	static bool bInited = false;
	if(!bInited) {
		// get rcon after all cfgs got executed
		bInited = true;
	}
}

void CSBGuardian::onMove(IPlayer *pPlayer) {
	// m_pSh->onMove(pPlayer);
}

void CSBGuardian::onTakeDamage(IPlayer *pVictim, IPlayer *pAttacker) {
	m_pAb->onHit(pVictim, pAttacker, hit_head);
}

void CSBGuardian::onJoinGame(IPlayer *pPlayer) {
	m_pFlash->onJoin(pPlayer);
	m_pAb->onJoin(pPlayer);
}

void CSBGuardian::onLeaveGame(IPlayer *pPlayer) {
	m_pFlash->onLeave(pPlayer);
}

void CSBGuardian::onCvarValue(IPlayer *pPlayer, const char *strName, const char *strValue, float flValue) {
	m_pCvar->onCvarValue(pPlayer, strName, strValue, flValue);
}

bool CSBGuardian::onCheckTransmit(IPlayer *pPlayer, IPlayer *pEnt) {
	if(pEnt == NULL || pPlayer == NULL)
		return true;

	if(pPlayer == pEnt)
		return true;

	if(!pPlayer->isInGame() || !pEnt->isInGame())
		return true;
#ifndef _PROFILE_
	if(pPlayer->isBot())
		return false;
	if(pEnt->isBot())
		return true;
#endif

	if(pPlayer->isSpec() || pPlayer->isHLTV() || pEnt->isSpec())
		return true;

	/* Deathcam is tricky! */
	if(pPlayer->isDying())
		return true;

	//bool alive = true;

	if(!pPlayer->isAlive())
	{
		/* 4 = First Person */
		if(pPlayer->specMode() != spec_fp)
			return true;

		IPlayer *p = m_pPlayerManager->getPlayer(pPlayer->getIdOfWatchedPlayer());
		if(p != NULL)
		{
			pPlayer = p;
	        	//alive = false;
		}
	}

	if(!pEnt->isAlive())
	{
		IPlayer *p = m_pPlayerManager->getPlayer(pEnt->getIdOfWatchedPlayer());
		if(p != NULL)
		{
			pEnt = p;
		}
	}

	/* TeamFilter */
	if(pPlayer->getTeam() == pEnt->getTeam())
		return true;
	if( m_pFlash->isFlashed(pPlayer) ) {
		// printf("%s is falshed!\n", pPlayer->getNick());
		return false;
	}

	if( !m_pFov->isInVision(pPlayer, pEnt) ) {
		// printf("%s cant see %s\n", pPlayer->getNick(), pEnt->getNick());
		return false;
	}

	if( !m_pWh->isVisible(pPlayer, pEnt) )
		return false;

	return true;
}

void CSBGuardian::onCommand() {
	const char *strArg1 = m_pEngine->argv(1);

	if( m_pEngine->argc() < 2 || strArg1 == NULL ) {
		notFound:
		printf("Commands:\nversion\nauthor\nurl\nstatus\n");
		return;
	}

	if( !strcmp(strArg1, "author") )
		printf("%s\n", AUTHOR);
	else if( !strcmp(strArg1, "version") )
		printf("version %s (rev%d)\ncompiled %s, %s\n", VERSION, REV, __DATE__, __TIME__);
	else if( !strcmp(strArg1, "url") )
		printf("%s\n", URL);
	else if( !strcmp(strArg1, "status") ) {
		printf("Module\t\tstatus\n");
		for(vector<IModule*>::iterator it = m_vecModule.begin(); it < m_vecModule.end(); ++it) {
			printf("%s:\t\t%s\n", (*it)->getName(), ((*it)->isEnabled())?"enabled":"disabled");
		}
		return;
	}
	else {
		for(vector<IModule*>::iterator it = m_vecModule.begin(); it < m_vecModule.end(); ++it) {
			if( (*it)->onCommand() )
				return;
		}

		goto notFound;
	}
}

bool CSBGuardian::onClientCommand(IPlayer *pPlayer) {
	if( !pPlayer->isInGame() )
		return true; // fix HalfConnected crash
	if( pPlayer->isBot() )
		return false;

	const char *strArg0 = m_pEngine->argv(0);
	if( strArg0 == NULL )
		return false;
	if( strcmp(strArg0, "sbguardian") )
		return false;

	const char *strArg1 = m_pEngine->argv(1);
	const char *strText = NULL;

	if( m_pEngine->argc() < 2 || strArg1 == NULL ) {
		notFound:
			strText = CStr::format("Commands:\nversion\nauthor\nurl\nstatus\n");
			pPlayer->printToConsole(strText);
			return true;
	}

	if( !strcmp(strArg1, "author") )
		strText = CStr::format("%s\n", AUTHOR);
	else if( !strcmp(strArg1, "version") )
		strText = CStr::format("version %s (rev%d)\ncompiled %s, %s\n", VERSION, REV, __DATE__, __TIME__);
	else if( !strcmp(strArg1, "url") )
		strText = CStr::format("%s\n", URL);
	else if( !strcmp(strArg1, "status") ) {
		strText = CStr::format("Module\t\tstatus\n");
		pPlayer->printToConsole(strText);
		for(vector<IModule*>::iterator it = m_vecModule.begin(); it < m_vecModule.end(); ++it) {
			strText = CStr::format("%s:\t\t%s\n", (*it)->getName(), ((*it)->isEnabled())?"enabled":"disabled");
			pPlayer->printToConsole(strText);
		}
		return true;
	}
	else {
		goto notFound;
	}

	if( strText != NULL ) {
		pPlayer->printToConsole(strText);
		return true;
	}

	return false;
}

void CSBGuardian::onPlayerFlash(IPlayer *pPlayer, float flTime) {
	m_pFlash->onFlash(pPlayer, flTime);
}

void CSBGuardian::onRoundStart() {
	m_pWm->onRestartRound();
}

void CSBGuardian::onEmitSound(vector<IPlayer *> &vecRecipients, int iEntIndex, int iChannel, float flVolume, float flAttenuation, CVector *vecOrigin) {
	IPlayer *pEnt = m_pPlayerManager->getPlayer(iEntIndex);
	if( pEnt == NULL )
		return;
	
	int iMaxClients = m_pPlayerManager->getMaxClients();
	for( int i = 1; i <= iMaxClients; i++ ) {
		IPlayer *pPlayer = m_pPlayerManager->getPlayer(i);
		if( pPlayer != NULL && vecRecipients.find(pPlayer) == NULL && m_pWh->transmitSound(pPlayer, pEnt) )
			vecRecipients.push_back(pPlayer);
	}
}
