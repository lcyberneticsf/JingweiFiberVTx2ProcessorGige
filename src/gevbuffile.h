/******************************************************************************
Copyright (c) 2018, Teledyne DALSA Inc.
All rights reserved.

File : gevbuffile.h
	Definitions for creating, reading, and writing ".gevbuf" container files.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions 
are met:
	-Redistributions of source code must retain the above copyright 
	notice, this list of conditions and the following disclaimer. 
	-Redistributions in binary form must reproduce the above 
	copyright notice, this list of conditions and the following 
	disclaimer in the documentation and/or other materials provided 
	with the distribution. 
	-Neither the name of Teledyne DALSA nor the names of its contributors 
	may be used to endorse or promote products derived 
	from this software without specific prior written permission. 

===============================================================================
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED 
OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/
/*! \file gevbuffile.h
\brief Definitions for access functions for ".gevbuf" container files.

*/
#ifndef __GEVBUFFILE_H__
#define __GEVBUFFILE_H__

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>

#include "stdio.h"
#include "cordef.h"
#include "gevapi.h"

//===========================================================
#ifndef htonll
	#define htonll(x) ((1 == htonl(1)) ? (x) : (((uint64_t)htonl((x) & 0xFFFFFFFFUL)) << 32) | htonl((uint32_t)((x) >> 32)))
#endif
#ifndef ntohll
	#define ntohll(x) ((1 == ntohl(1)) ? (x) : (((uint64_t)ntohl((x) & 0xFFFFFFFFUL)) << 32) | ntohl((uint32_t)((x) >> 32)))
#endif

#define MAGIC_GEVBUF	0x47455642   // Magic number "GEVB"

typedef struct __attribute__((packed)) _tag_GEVBUFFILE
{
	UINT32 payload_type;	// Type of payload received 
								// Image specific payload entries.
	UINT32 x_offset;		// Received x offset for origin of ROI for an image payload_type.
	UINT32 y_offset;		// Received y offset for origin of ROI for an image payload_type.
	UINT32 h;				// Received height (lines) for an image payload. 
	UINT32 w;				// Received width (pixels) for an image payload.
	UINT32 format;			// Received Gige Vision pixel format for image or JPEG data types.
								// (If the format value is not a valid Gige Vision pixel type and the payload_type is JPEG, then the format value 
								//  is to be interpreted as a color space value (EnumCS value for JPEG) to be used by a JPEG decoder).
								//
	UINT64 id;				// Device level Block id (index) for the payload (starts at 1, may wrap to 1 at 65535)
	UINT64 timestamp; 	// Timestamp for payload using device level timebase.
	UINT64 recv_size;		// Received size of entire payload (including all appended "chunk" (metadata) information) . 
	UINT64 chunk_offset;	// Chunk information entry - payload_type dependent :
								// Payloads of type "Image"  : The offset of "chunk" data (metadata) associated with the received payload 
								//	                            (0 if no "chunk" data (metadata) is available).
								// Payloads of type "Turbo*" : The uncompressed size of the "chunk" data (metadata) associated with the 
								//                             received compressed payload. (0 if no "chunk" data (metadata) is available).
	UINT8  data[1];		// "recv_size" data starts here (with image metadata located at "chunk_offset" from this point.
} GEVBUFFILE, *PGEVBUFFILE;

// Container payload types.
#define GEV_PAYLOAD_TYPE_IMAGE                       0x0001
#define GEV_PAYLOAD_TYPE_EXTD_CHUNK                  0x0005
#define GEV_PAYLOAD_TYPE_RAW_DATA                    0x0002
#define GEV_PAYLOAD_TYPE_RAW_DATA_AND_CHUNK          0x4002
#define GEV_PAYLOAD_TYPE_TURBO_MODE                  0x8002
#define GEV_PAYLOAD_TYPE_TURBO_MODE_AND_CHUNK        0xC002
#define GEV_PAYLOAD_TYPE_TURBO_MODE_BAYER            0x8003
#define GEV_PAYLOAD_TYPE_TURBO_MODE_BAYER_AND_CHUNK  0xC003

static inline int IsRawTurboModeData( UINT32 payload_type )
{
	switch(payload_type)
	{
		case GEV_PAYLOAD_TYPE_TURBO_MODE:
		case GEV_PAYLOAD_TYPE_TURBO_MODE_AND_CHUNK:
		case GEV_PAYLOAD_TYPE_TURBO_MODE_BAYER:
		case GEV_PAYLOAD_TYPE_TURBO_MODE_BAYER_AND_CHUNK:
			return TRUE;
			break;
		default:
			break;
	}
	return FALSE;
}

#ifdef __cplusplus
extern "C" {
#endif

unsigned long GEVBUFFILE_GetFileSize( char *filename );
unsigned int GEVBUFFILE_GetTotalFrames( char *filename );

int GEVBUFFILE_SaveSingleBufferObject( char *filename, GEV_BUFFER_OBJECT *buf );

FILE* GEVBUFFILE_Create( char *filename );
unsigned long	GEVBUFFILE_AddFrame( FILE *fp, GEV_BUFFER_OBJECT *frame );
int 	GEVBUFFILE_Close( FILE *fp, unsigned int total_frames );


unsigned long GEVBUFFILE_RestoreFrameData( char *filename, unsigned long data_size, GEVBUFFILE *first_frame, void **end_marker );
GEVBUFFILE *GEVBUFFILE_GetNextFrame( GEVBUFFILE *previous_frame, void *end_marker );


#ifdef __cplusplus
}
#endif


#endif
