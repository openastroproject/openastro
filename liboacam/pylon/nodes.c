/*****************************************************************************
 *
 * nodes.c -- Basler Pylon node-handling functions
 *
 * Copyright 2020,2021
 *   James Fidell (james@openastroproject.org)
 *
 * License:
 *
 * This file is part of the Open Astro Project.
 *
 * The Open Astro Project is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Open Astro Project is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Open Astro Project.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************/

#include <oa_common.h>

#include <pylonc/PylonC.h>

#include <openastro/errno.h>
#include <openastro/util.h>

#include "private.h"
#include "nodes.h"


int
_pylonGetEnumerationNode ( NODEMAP_HANDLE nodeMap, const char* nodeName,
		int readWrite, NODE_HANDLE* node )
{
	EGenApiNodeType		nodeType;
	_Bool							res;

	if (( p_GenApiNodeMapGetNode )( nodeMap, nodeName, node ) != GENAPI_E_OK ) {
    oaLogError ( OA_LOG_CAMERA, "%s: GenApiNodeMapGetNode ('%s') failed",
				__func__, nodeName );
    return -OA_ERR_SYSTEM_ERROR;
  }

	if (( p_GenApiNodeGetType )( *node, &nodeType ) != GENAPI_E_OK ) {
		oaLogError ( OA_LOG_CAMERA, "%s: GenApiNodeGetType ('%s') failed",
				__func__, nodeName );
		return -OA_ERR_SYSTEM_ERROR;
	}
	if ( EnumerationNode != nodeType ) {
		oaLogError ( OA_LOG_CAMERA, "%s: '%s' is not an enumeration node",
				__func__, nodeName );
		return -OA_ERR_INVALID_CONTROL_TYPE;
	}

	if ( readWrite ) {
		if (( p_GenApiNodeIsReadable )( *node, &res ) != GENAPI_E_OK ) {
			oaLogError ( OA_LOG_CAMERA, "%s: GenApiNodeIsReadable ('%s') failed",
					__func__, nodeName );
			return -OA_ERR_SYSTEM_ERROR;
		}
		if ( !res ) {
			oaLogError ( OA_LOG_CAMERA, "%s: node %s is not readable", __func__,
					nodeName );
			return -OA_ERR_NOT_READABLE;
		}
	}

	if ( readWrite & 0x2 ) {
		if (( p_GenApiNodeIsWritable )( *node, &res ) != GENAPI_E_OK ) {
			oaLogError ( OA_LOG_CAMERA, "%s: GenApiNodeIsWritable ('%s') failed",
					__func__, nodeName );
			return -OA_ERR_SYSTEM_ERROR;
		}
		if ( !res ) {
			oaLogError ( OA_LOG_CAMERA, "%s: node %s is not writeable", __func__,
					nodeName );
			return -OA_ERR_NOT_WRITEABLE;
		}
	}

	return OA_ERR_NONE;
}


int
_pylonGetBooleanNode ( NODEMAP_HANDLE nodeMap, const char* nodeName,
		int readWrite, NODE_HANDLE* node )
{
	EGenApiNodeType		nodeType;
	_Bool							res;

	if (( p_GenApiNodeMapGetNode )( nodeMap, nodeName, node ) != GENAPI_E_OK ) {
    oaLogError ( OA_LOG_CAMERA, "%s: GenApiNodeMapGetNode ('%s') failed",
				__func__, nodeName );
    return -OA_ERR_SYSTEM_ERROR;
  }

	if (( p_GenApiNodeGetType )( *node, &nodeType ) != GENAPI_E_OK ) {
		oaLogError ( OA_LOG_CAMERA, "%s: GenApiNodeGetType ('%s') failed",
				__func__, nodeName );
		return -OA_ERR_SYSTEM_ERROR;
	}
	if ( BooleanNode != nodeType ) {
		oaLogError ( OA_LOG_CAMERA, "%s: '%s' is not a boolean node: %d", __func__,
				nodeName, nodeType );
		return -OA_ERR_INVALID_CONTROL_TYPE;
	}

	if ( readWrite & 0x1 ) {
		if (( p_GenApiNodeIsReadable )( *node, &res ) != GENAPI_E_OK ) {
			oaLogError ( OA_LOG_CAMERA, "%s: GenApiNodeIsReadable ('%s') failed",
					__func__, nodeName );
			return -OA_ERR_SYSTEM_ERROR;
		}
		if ( !res ) {
			oaLogError ( OA_LOG_CAMERA, "%s: node %s is not readable", __func__,
					nodeName );
			return -OA_ERR_NOT_READABLE;
		}
	}

	if ( readWrite & 0x2 ) {
		if (( p_GenApiNodeIsWritable )( *node, &res ) != GENAPI_E_OK ) {
			oaLogError ( OA_LOG_CAMERA, "%s: GenApiNodeIsWritable ('%s') failed",
					__func__, nodeName );
			return -OA_ERR_SYSTEM_ERROR;
		}
		if ( !res ) {
			oaLogError ( OA_LOG_CAMERA, "%s: node %s is not writeable", __func__,
					nodeName );
			return -OA_ERR_NOT_WRITEABLE;
		}
	}

	return OA_ERR_NONE;
}


int
_pylonGetIntNode ( NODEMAP_HANDLE nodeMap, const char* nodeName,
		int readWrite, NODE_HANDLE* node )
{
	EGenApiNodeType		nodeType;
	_Bool							res;

	if (( p_GenApiNodeMapGetNode )( nodeMap, nodeName, node ) != GENAPI_E_OK ) {
    oaLogError ( OA_LOG_CAMERA, "%s: GenApiNodeMapGetNode ('%s') failed",
				__func__, nodeName );
    return -OA_ERR_SYSTEM_ERROR;
  }

	if (( p_GenApiNodeGetType )( *node, &nodeType ) != GENAPI_E_OK ) {
		oaLogError ( OA_LOG_CAMERA, "%s: GenApiNodeGetType ('%s') failed",
				__func__, nodeName );
		return -OA_ERR_SYSTEM_ERROR;
	}
	if ( IntegerNode != nodeType ) {
		oaLogError ( OA_LOG_CAMERA, "%s: '%s' is not an integer node: %d",
				__func__, nodeName, nodeType );
		return -OA_ERR_INVALID_CONTROL_TYPE;
	}

	if ( readWrite & 0x1 ) {
		if (( p_GenApiNodeIsReadable )( *node, &res ) != GENAPI_E_OK ) {
			oaLogError ( OA_LOG_CAMERA, "%s: GenApiNodeIsReadable ('%s') failed",
					__func__, nodeName );
			return -OA_ERR_SYSTEM_ERROR;
		}
		if ( !res ) {
			oaLogError ( OA_LOG_CAMERA, "%s: node %s is not readable", __func__,
					nodeName );
			return -OA_ERR_NOT_READABLE;
		}
	}

	if ( readWrite & 0x2 ) {
		if (( p_GenApiNodeIsWritable )( *node, &res ) != GENAPI_E_OK ) {
			oaLogError ( OA_LOG_CAMERA, "%s: GenApiNodeIsWritable ('%s') failed",
					__func__, nodeName );
			return -OA_ERR_SYSTEM_ERROR;
		}
		if ( !res ) {
			oaLogError ( OA_LOG_CAMERA, "%s: node %s is not writeable", __func__,
					nodeName );
			return -OA_ERR_NOT_WRITEABLE;
		}
	}

	return OA_ERR_NONE;
}


int
_pylonGetFloatNode ( NODEMAP_HANDLE nodeMap, const char* nodeName,
		int readWrite, NODE_HANDLE* node )
{
	EGenApiNodeType		nodeType;
	_Bool							res;

	if (( p_GenApiNodeMapGetNode )( nodeMap, nodeName, node ) != GENAPI_E_OK ) {
    oaLogError ( OA_LOG_CAMERA, "%s: GenApiNodeMapGetNode ('%s') failed",
				__func__, nodeName );
    return -OA_ERR_SYSTEM_ERROR;
  }

	if (( p_GenApiNodeGetType )( *node, &nodeType ) != GENAPI_E_OK ) {
		oaLogError ( OA_LOG_CAMERA, "%s: GenApiNodeGetType ('%s') failed",
				__func__, nodeName );
		return -OA_ERR_SYSTEM_ERROR;
	}
	if ( FloatNode != nodeType ) {
		oaLogError ( OA_LOG_CAMERA, "%s: '%s' is not a float node: %d", __func__,
				nodeName, nodeType );
		return -OA_ERR_INVALID_CONTROL_TYPE;
	}

	if ( readWrite & 0x1 ) {
		if (( p_GenApiNodeIsReadable )( *node, &res ) != GENAPI_E_OK ) {
			oaLogError ( OA_LOG_CAMERA, "%s: GenApiNodeIsReadable ('%s') failed",
					__func__, nodeName );
			return -OA_ERR_SYSTEM_ERROR;
		}
		if ( !res ) {
			oaLogError ( OA_LOG_CAMERA, "%s: node %s is not readable", __func__,
					nodeName );
			return -OA_ERR_NOT_READABLE;
		}
	}

	if ( readWrite & 0x2 ) {
		if (( p_GenApiNodeIsWritable )( *node, &res ) != GENAPI_E_OK ) {
			oaLogError ( OA_LOG_CAMERA, "%s: GenApiNodeIsWritable ('%s') failed",
					__func__, nodeName );
			return -OA_ERR_SYSTEM_ERROR;
		}
		if ( !res ) {
			oaLogError ( OA_LOG_CAMERA, "%s: node %s is not writeable", __func__,
					nodeName );
			return -OA_ERR_NOT_WRITEABLE;
		}
	}

	return OA_ERR_NONE;
}


void
_pylonShowEnumValues ( NODE_HANDLE node, const char* nodeName )
{
	size_t				numEnums, i, len;
	NODE_HANDLE		enumNode;
	char					val1[ 128 ], val2[128];

	if (( p_GenApiEnumerationGetNumEntries )( node, &numEnums ) !=
			GENAPI_E_OK ) {
		oaLogError ( OA_LOG_CAMERA,
				"%s: GenApiEnumerationGetNumEntries ('%s') failed", __func__,
				nodeName );
		return;
	}

	printf ( "Values for '%s':\n", nodeName );
	for ( i = 0; i < numEnums; i++ ) {
		if (( p_GenApiEnumerationGetEntryByIndex )( node, i, &enumNode ) !=
				GENAPI_E_OK ) {
			oaLogError ( OA_LOG_CAMERA,
					"%s: GenApiEnumerationGetEntryByIndex ('%s') failed", __func__,
					nodeName );
			return;
		}
		len = sizeof ( val1 );
		if (( p_GenApiNodeGetName )( enumNode, val1, &len ) !=
				GENAPI_E_OK ) {
			oaLogError ( OA_LOG_CAMERA, "%s: GenApiNodeGetName ('%s') failed",
					__func__, nodeName );
			return;
		}
		len = sizeof ( val2 );
		if (( p_GenApiNodeGetDisplayName )( enumNode, val2, &len ) !=
				GENAPI_E_OK ) {
			oaLogError ( OA_LOG_CAMERA, "%s: GenApiNodeGetDisplayName ('%s') failed",
					__func__, nodeName );
			return;
		}
		printf ( "  %s (%s)\n", val1, val2 );
	}
}


int
_pylonGetIntSettings ( NODE_HANDLE node, int64_t* pmin, int64_t* pmax,
		int64_t* pstep, int64_t* pcurr )
{
	if (( p_GenApiIntegerGetMin )( node, pmin ) != GENAPI_E_OK ) {
		oaLogError ( OA_LOG_CAMERA, "%s: p_GenApiIntegerGetMin failed", __func__ );
		return -OA_ERR_SYSTEM_ERROR;
	}
	if (( p_GenApiIntegerGetMax )( node, pmax ) != GENAPI_E_OK ) {
		oaLogError ( OA_LOG_CAMERA, "%s: p_GenApiIntegerGetMax failed", __func__ );
		return -OA_ERR_SYSTEM_ERROR;
	}
	if (( p_GenApiIntegerGetInc )( node, pstep ) != GENAPI_E_OK ) {
		oaLogError ( OA_LOG_CAMERA, "%s: p_GenApiIntegerGetInc failed", __func__ );
		return -OA_ERR_SYSTEM_ERROR;
	}
	if (( p_GenApiIntegerGetValue )( node, pcurr ) != GENAPI_E_OK ) {
		oaLogError ( OA_LOG_CAMERA, "%s: p_GenApiIntegerGetValue failed",
				__func__ );
		return -OA_ERR_SYSTEM_ERROR;
	}

	return OA_ERR_NONE;
}


int
_pylonGetFloatSettings ( NODE_HANDLE node, double* pmin, double* pmax,
		double* pcurr )
{
	if (( p_GenApiFloatGetMin )( node, pmin ) != GENAPI_E_OK ) {
		oaLogError ( OA_LOG_CAMERA, "%s: p_GenApiFloatGetMin failed", __func__ );
		return -OA_ERR_SYSTEM_ERROR;
	}
	if (( p_GenApiFloatGetMax )( node, pmax ) != GENAPI_E_OK ) {
		oaLogError ( OA_LOG_CAMERA, "%s: p_GenApiFloatGetMax failed", __func__ );
		return -OA_ERR_SYSTEM_ERROR;
	}
	if (( p_GenApiFloatGetValue )( node, pcurr ) != GENAPI_E_OK ) {
		oaLogError ( OA_LOG_CAMERA, "%s: p_GenApiFloatGetValue failed", __func__ );
		return -OA_ERR_SYSTEM_ERROR;
	}

	return OA_ERR_NONE;
}
