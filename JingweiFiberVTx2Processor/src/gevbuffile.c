/******************************************************************************
Copyright (c) 2018, Teledyne DALSA Inc.
All rights reserved.

File : gevbuffile.c
	Functions for creating, reading, and writing ".gevbuf" container files.

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
/*! \file gevbuffile.c
\brief Access functions for ".gevbuf" container files.

*/

#include "gevbuffile.h"

unsigned int GEVBUFFILE_GetTotalFrames( char *filename )
{
	UINT32 total_frames = 0;
	if ( filename != NULL )
	{
		// Data will fit in buffer passed in.
		FILE *fp = NULL;
		fp = fopen(filename, "rb");
		if (fp != NULL)
		{
			UINT32 magic = 0;
			// Seek to the end of the file.
			fseek( fp, (-2*(sizeof(UINT32))), SEEK_END);
			fread((void *)&magic, sizeof(magic), 1, fp);
			fread((void *)&total_frames, sizeof(total_frames), 1, fp);
			fclose(fp);
			
			if ( MAGIC_GEVBUF != ntohl(magic))
			{
				// No magic number (file is not correct).
				total_frames = 0;
			}			
		}
	}
	return (unsigned int)total_frames;
}

unsigned long GEVBUFFILE_GetFileSize( char *filename )
{
	unsigned long fsize = 0;
	UINT32 total_frames = 0;
	if ( filename != NULL )
	{
		struct stat st;
				
		stat(filename, &st);
		fsize = st.st_size;		
		
		total_frames = GEVBUFFILE_GetTotalFrames( filename );
		if (total_frames == 0)
		{
			// File is not valid - report it is empty.
			fsize = 0;
		}
	}
	return fsize;
}

unsigned long GEVBUFFILE_RestoreFrameData( char *filename, unsigned long data_size, GEVBUFFILE *first_frame, void **end_marker )
{
	unsigned long fsize = 0;
	
	if ( (filename != NULL) && (first_frame != NULL)  && (end_marker != NULL) )
	{
		unsigned char *base = (unsigned char *)first_frame;
		unsigned char *end = NULL;
		fsize = GEVBUFFILE_GetFileSize(filename);
		
		if ( data_size >= fsize )
		{
			// Data will fit in buffer passed in.
			FILE *fp = NULL;
				
			fp = fopen(filename, "rb"); 
								
			if (fp != NULL)
			{
				// Read all the frame data.
				fread((void *)first_frame, 1, (size_t)fsize, fp);
				
				// Done with the file.
				fclose(fp);
				
				// Set up the information for the first frame in the file.
				// Restore the header "endian-ness"
				first_frame->payload_type = ntohl(first_frame->payload_type);
				first_frame->timestamp = ntohll(first_frame->timestamp);
				first_frame->recv_size = ntohll(first_frame->recv_size);
				first_frame->id = ntohll(first_frame->id);
				first_frame->x_offset = ntohl(first_frame->x_offset);
				first_frame->y_offset = ntohl(first_frame->y_offset);
				first_frame->h = ntohl(first_frame->h);
				first_frame->w = ntohl(first_frame->w);
				first_frame->format = ntohl(first_frame->format);
				first_frame->chunk_offset = ntohll(first_frame->chunk_offset);
				
				// Update the end_marker.
				end = base + fsize;
				*end_marker = (void *)end;

				// Update the size read to that of the first cached frame container.
				fsize = sizeof(GEVBUFFILE) - 1 + first_frame->recv_size;
				
				// Restore the "endian-ness" to all loaded frames.
				{
					unsigned long frame_size = fsize;
					unsigned char *next_frame = base + frame_size;

					while ( (next_frame + sizeof(GEVBUFFILE)) < end )
					{
						GEVBUFFILE *fdata = (GEVBUFFILE *)next_frame;
						fdata->payload_type = ntohl(fdata->payload_type);
						fdata->timestamp = ntohll(fdata->timestamp);
						fdata->recv_size = ntohll(fdata->recv_size);
						fdata->id = ntohll(fdata->id);
						fdata->x_offset = ntohl(fdata->x_offset);
						fdata->y_offset = ntohl(fdata->y_offset);
						fdata->h = ntohl(fdata->h);
						fdata->w = ntohl(fdata->w);
						fdata->format = ntohl(fdata->format);					
						fdata->chunk_offset = ntohll(fdata->chunk_offset);
						
						// Update to next frame.
						frame_size = sizeof(GEVBUFFILE) - 1 + fdata->recv_size;
						next_frame = (unsigned char *)fdata + frame_size;
					}
				}
			}
			else
			{
				//No data read
				fsize = 0;
			}
		}
		else
		{
			// No data read.
			fsize = 0;
		}
	}
	return (int)fsize;
}

GEVBUFFILE *GEVBUFFILE_GetNextFrame( GEVBUFFILE *previous_frame, void *end_marker )
{
	if (previous_frame != NULL)
	{
		unsigned long frame_size = sizeof(GEVBUFFILE) - 1 + previous_frame->recv_size;
		unsigned char *next_frame = (unsigned char *)previous_frame + frame_size;
		if ( (next_frame + sizeof(GEVBUFFILE)) < (unsigned char *)end_marker )
		{
			return (GEVBUFFILE *)next_frame;
		}
		else
		{
			return NULL;
		}
	}
	return NULL;	
} 

// Save a single frame in a .gevbuf file.
int GEVBUFFILE_SaveSingleBufferObject( char *filename, GEV_BUFFER_OBJECT *buf )
{
	int status = -1;
	if ((filename != NULL) && (buf != NULL))
	{
		FILE *fp = NULL;
		fp = fopen(filename, "wb"); 
						
		if (fp != NULL)
		{
			GEVBUFFILE fdata = {0};
			unsigned char *data_address  = buf->address;
			UINT64 chunk_address = (size_t) buf->chunk_data;
				
			fdata.payload_type = htonl(buf->payload_type);
			fdata.timestamp = htonll(buf->timestamp);
			fdata.recv_size = htonll(buf->recv_size);
			fdata.id = htonll(buf->id);
			fdata.x_offset = htonl(buf->x_offset);
			fdata.y_offset = htonl(buf->y_offset);
			fdata.h = htonl(buf->h);
			fdata.w = htonl(buf->w);
			fdata.format = htonl(buf->format);
			if (buf->chunk_size != 0)
			{
				if (IsRawTurboModeData( buf->payload_type ) )
				{
					fdata.chunk_offset = htonll( (UINT64)buf->chunk_size);
				}
				else
				{
					fdata.chunk_offset = htonll( (UINT64)(chunk_address - (size_t)data_address));
				}
			}
			else
			{
				fdata.chunk_offset = 0;
			}
			
			// Write
			{
				unsigned long hdr_size = sizeof(GEVBUFFILE) - 1;
				unsigned int magic = htonl(MAGIC_GEVBUF);
				UINT32 total_frames = 1;
				fwrite( &fdata, sizeof(unsigned char), (size_t)hdr_size, fp);
				fwrite( data_address, sizeof(unsigned char), (size_t)buf->recv_size, fp);
				fwrite( &magic, sizeof(magic), 1, fp);
				fwrite( &total_frames, sizeof(total_frames), 1, fp);
			}
								
			fclose(fp);
		}
	}
	return status;
}

FILE* GEVBUFFILE_Create( char *filename )
{
	if ( filename != NULL )
	{
		FILE *fp = fopen(filename, "wb");
		return fp;
	}
	else
	{
		return NULL;
	}
}

int 	GEVBUFFILE_Close( FILE *fp, unsigned int total_frames )
{
	if ( fp != NULL )
	{
		unsigned int magic = htonl(MAGIC_GEVBUF);
		fwrite( &magic, sizeof(magic), 1, fp);
		fwrite( &total_frames, sizeof(total_frames), 1, fp);
		return fclose(fp);
	}
	return -1;
}


unsigned long GEVBUFFILE_AddFrame( FILE *fp, GEV_BUFFER_OBJECT *frame )
{
	unsigned long written = 0;
	if (fp != NULL)
	{
		GEVBUFFILE fdata = {0};
		unsigned char *data_address  = frame->address;
		unsigned char *chunk_address = frame->chunk_data;
				
		fdata.payload_type = htonl(frame->payload_type);
		fdata.timestamp = htonll(frame->timestamp);
		fdata.recv_size = htonll(frame->recv_size);
		fdata.id = htonll(frame->id);
		fdata.x_offset = htonl(frame->x_offset);
		fdata.y_offset = htonl(frame->y_offset);
		fdata.h = htonl(frame->h);
		fdata.w = htonl(frame->w);
		fdata.format = htonl(frame->format);
		if (frame->chunk_size != 0)
		{
			if (IsRawTurboModeData( frame->payload_type ) )
			{
				fdata.chunk_offset = htonll( (UINT64)frame->chunk_size);
			}
			else
			{
				fdata.chunk_offset = htonll( (UINT64)(chunk_address - data_address));
			}
		}
		else
		{
			fdata.chunk_offset = 0;
		}
			
		// Write
		{
			unsigned long hdr_size = sizeof(GEVBUFFILE) - 1;
			written += (unsigned long) fwrite( &fdata, sizeof(unsigned char), (size_t)hdr_size, fp);
			written += (unsigned long) fwrite( data_address, sizeof(unsigned char), (size_t)frame->recv_size, fp);
		}
	}
	return written;
	
}




