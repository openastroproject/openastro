/*****************************************************************************
 *
 * nodes.c -- Basler Pylon node-handling functions
 *
 * Copyright 2020
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

#include "private.h"
#include "nodes.h"


int
_pylonGetEnumerationNode ( NODEMAP_HANDLE nodeMap, const char* nodeName,
		int readWrite, NODE_HANDLE* node )
{
	EGenApiNodeType		nodeType;
	_Bool							res;

	if (( p_GenApiNodeMapGetNode )( nodeMap, nodeName, node ) != GENAPI_E_OK ) {
    fprintf ( stderr, "GenApiNodeMapGetNode ('%s') failed\n", nodeName );
    return -OA_ERR_SYSTEM_ERROR;
  }

	if (( p_GenApiNodeGetType )( *node, &nodeType ) != GENAPI_E_OK ) {
		fprintf ( stderr, "GenApiNodeGetType ('%s') failed\n", nodeName );
		return -OA_ERR_SYSTEM_ERROR;
	}
	if ( EnumerationNode != nodeType ) {
		fprintf ( stderr, "'%s' is not an enumeration node\n", nodeName );
		return -OA_ERR_INVALID_CONTROL_TYPE;
	}

	if ( readWrite ) {
		if (( p_GenApiNodeIsReadable )( *node, &res ) != GENAPI_E_OK ) {
			fprintf ( stderr, "GenApiNodeIsReadable ('%s') failed\n", nodeName );
			return -OA_ERR_SYSTEM_ERROR;
		}
		if ( !res ) {
			fprintf ( stderr, "node %s is not readable\n", nodeName );
			return -OA_ERR_NOT_READABLE;
		}
	}

	if ( readWrite & 0x2 ) {
		if (( p_GenApiNodeIsWritable )( *node, &res ) != GENAPI_E_OK ) {
			fprintf ( stderr, "GenApiNodeIsWritable ('%s') failed\n", nodeName );
			return -OA_ERR_SYSTEM_ERROR;
		}
		if ( !res ) {
			fprintf ( stderr, "node %s is not writeable\n", nodeName );
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
    fprintf ( stderr, "GenApiNodeMapGetNode ('%s') failed\n", nodeName );
    return -OA_ERR_SYSTEM_ERROR;
  }

	if (( p_GenApiNodeGetType )( *node, &nodeType ) != GENAPI_E_OK ) {
		fprintf ( stderr, "GenApiNodeGetType ('%s') failed\n", nodeName );
		return -OA_ERR_SYSTEM_ERROR;
	}
	if ( BooleanNode != nodeType ) {
		fprintf ( stderr, "'%s' is not a boolean node: %d\n", nodeName, nodeType );
		return -OA_ERR_INVALID_CONTROL_TYPE;
	}

	if ( readWrite & 0x1 ) {
		if (( p_GenApiNodeIsReadable )( *node, &res ) != GENAPI_E_OK ) {
			fprintf ( stderr, "GenApiNodeIsReadable ('%s') failed\n", nodeName );
			return -OA_ERR_SYSTEM_ERROR;
		}
		if ( !res ) {
			fprintf ( stderr, "node %s is not readable\n", nodeName );
			return -OA_ERR_NOT_READABLE;
		}
	}

	if ( readWrite & 0x2 ) {
		if (( p_GenApiNodeIsWritable )( *node, &res ) != GENAPI_E_OK ) {
			fprintf ( stderr, "GenApiNodeIsWritable ('%s') failed\n", nodeName );
			return -OA_ERR_SYSTEM_ERROR;
		}
		if ( !res ) {
			fprintf ( stderr, "node %s is not writeable\n", nodeName );
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
    fprintf ( stderr, "GenApiNodeMapGetNode ('%s') failed\n", nodeName );
    return -OA_ERR_SYSTEM_ERROR;
  }

	if (( p_GenApiNodeGetType )( *node, &nodeType ) != GENAPI_E_OK ) {
		fprintf ( stderr, "GenApiNodeGetType ('%s') failed\n", nodeName );
		return -OA_ERR_SYSTEM_ERROR;
	}
	if ( IntegerNode != nodeType ) {
		fprintf ( stderr, "'%s' is not an integer node: %d\n", nodeName, nodeType );
		return -OA_ERR_INVALID_CONTROL_TYPE;
	}

	if ( readWrite & 0x1 ) {
		if (( p_GenApiNodeIsReadable )( *node, &res ) != GENAPI_E_OK ) {
			fprintf ( stderr, "GenApiNodeIsReadable ('%s') failed\n", nodeName );
			return -OA_ERR_SYSTEM_ERROR;
		}
		if ( !res ) {
			fprintf ( stderr, "node %s is not readable\n", nodeName );
			return -OA_ERR_NOT_READABLE;
		}
	}

	if ( readWrite & 0x2 ) {
		if (( p_GenApiNodeIsWritable )( *node, &res ) != GENAPI_E_OK ) {
			fprintf ( stderr, "GenApiNodeIsWritable ('%s') failed\n", nodeName );
			return -OA_ERR_SYSTEM_ERROR;
		}
		if ( !res ) {
			fprintf ( stderr, "node %s is not writeable\n", nodeName );
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
    fprintf ( stderr, "GenApiNodeMapGetNode ('%s') failed\n", nodeName );
    return -OA_ERR_SYSTEM_ERROR;
  }

	if (( p_GenApiNodeGetType )( *node, &nodeType ) != GENAPI_E_OK ) {
		fprintf ( stderr, "GenApiNodeGetType ('%s') failed\n", nodeName );
		return -OA_ERR_SYSTEM_ERROR;
	}
	if ( FloatNode != nodeType ) {
		fprintf ( stderr, "'%s' is not a float node: %d\n", nodeName, nodeType );
		return -OA_ERR_INVALID_CONTROL_TYPE;
	}

	if ( readWrite & 0x1 ) {
		if (( p_GenApiNodeIsReadable )( *node, &res ) != GENAPI_E_OK ) {
			fprintf ( stderr, "GenApiNodeIsReadable ('%s') failed\n", nodeName );
			return -OA_ERR_SYSTEM_ERROR;
		}
		if ( !res ) {
			fprintf ( stderr, "node %s is not readable\n", nodeName );
			return -OA_ERR_NOT_READABLE;
		}
	}

	if ( readWrite & 0x2 ) {
		if (( p_GenApiNodeIsWritable )( *node, &res ) != GENAPI_E_OK ) {
			fprintf ( stderr, "GenApiNodeIsWritable ('%s') failed\n", nodeName );
			return -OA_ERR_SYSTEM_ERROR;
		}
		if ( !res ) {
			fprintf ( stderr, "node %s is not writeable\n", nodeName );
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
		fprintf ( stderr, "GenApiEnumerationGetNumEntries ('%s') failed\n",
				nodeName );
		return;
	}

	printf ( "Values for '%s':\n", nodeName );
	for ( i = 0; i < numEnums; i++ ) {
		if (( p_GenApiEnumerationGetEntryByIndex )( node, i, &enumNode ) !=
				GENAPI_E_OK ) {
			fprintf ( stderr, "GenApiEnumerationGetEntryByIndex ('%s') failed\n",
					nodeName );
			return;
		}
		len = sizeof ( val1 );
		if (( p_GenApiNodeGetName )( enumNode, val1, &len ) !=
				GENAPI_E_OK ) {
			fprintf ( stderr, "GenApiNodeGetName ('%s') failed\n",
					nodeName );
			return;
		}
		len = sizeof ( val2 );
		if (( p_GenApiNodeGetDisplayName )( enumNode, val2, &len ) !=
				GENAPI_E_OK ) {
			fprintf ( stderr, "GenApiNodeGetDisplayName ('%s') failed\n",
					nodeName );
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
		fprintf ( stderr, "p_GenApiIntegerGetMin failed\n" );
		return -OA_ERR_SYSTEM_ERROR;
	}
	if (( p_GenApiIntegerGetMax )( node, pmax ) != GENAPI_E_OK ) {
		fprintf ( stderr, "p_GenApiIntegerGetMax failed\n" );
		return -OA_ERR_SYSTEM_ERROR;
	}
	if (( p_GenApiIntegerGetInc )( node, pstep ) != GENAPI_E_OK ) {
		fprintf ( stderr, "p_GenApiIntegerGetInc failed\n" );
		return -OA_ERR_SYSTEM_ERROR;
	}
	if (( p_GenApiIntegerGetValue )( node, pcurr ) != GENAPI_E_OK ) {
		fprintf ( stderr, "p_GenApiIntegerGetValue failed\n" );
		return -OA_ERR_SYSTEM_ERROR;
	}

	return OA_ERR_NONE;
}


int
_pylonGetFloatSettings ( NODE_HANDLE node, double* pmin, double* pmax,
		double* pcurr )
{
	if (( p_GenApiFloatGetMin )( node, pmin ) != GENAPI_E_OK ) {
		fprintf ( stderr, "p_GenApiFloatGetMin failed\n" );
		return -OA_ERR_SYSTEM_ERROR;
	}
	if (( p_GenApiFloatGetMax )( node, pmax ) != GENAPI_E_OK ) {
		fprintf ( stderr, "p_GenApiFloatGetMax failed\n" );
		return -OA_ERR_SYSTEM_ERROR;
	}
	if (( p_GenApiFloatGetValue )( node, pcurr ) != GENAPI_E_OK ) {
		fprintf ( stderr, "p_GenApiFloatGetValue failed\n" );
		return -OA_ERR_SYSTEM_ERROR;
	}

	return OA_ERR_NONE;
}
