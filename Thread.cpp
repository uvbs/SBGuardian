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

#include "Thread.h"

bool CThreads::newThread(unsigned long int *iId, funcThread pFunc, void *argv) {
#ifdef WIN32
	*iId = (int) CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) pFunc, argv, 0, NULL);
#else
	pthread_create(iId, NULL, pFunc, argv);
#endif
	return true;
}

void CThreads::waitForThread(unsigned long int iId) {
#ifdef WIN32
	WaitForSingleObject((void *) iId, 0);
#else
	pthread_join(iId, NULL);
#endif
}