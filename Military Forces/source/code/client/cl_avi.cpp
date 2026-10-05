/*
===========================================================================
Copyright (C) 2005-2006 Tim Angus

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "client.h"
#include "snd_local.h"

#define INDEX_FILE_EXTENSION ".index.dat"

constexpr int MAX_RIFF_CHUNKS = 16;

struct audioFormat_s
{
  int rate;
  int format;
  int channels;
  int bits;

  int sampleSize;
  int totalBytes;
};
using audioFormat_t = audioFormat_s;

struct aviFileData_s
{
  bool      fileOpen;
  fileHandle_t  f;
  char          fileName[ MAX_QPATH ];
  int           fileSize;
  int           moviOffset;
  int           moviSize;

  fileHandle_t  idxF;
  int           numIndices;

  int           frameRate;
  int           framePeriod;
  int           width, height;
  int           numVideoFrames;
  int           maxRecordSize;
  bool      motionJpeg;

  bool      audio;
  audioFormat_t a;
  int           numAudioFrames;

  int           chunkStack[ MAX_RIFF_CHUNKS ];
  int           chunkStackTop;

  byte          *cBuffer, *eBuffer;
};
using aviFileData_t = aviFileData_s;

static aviFileData_t afd;

constexpr int MAX_AVI_BUFFER = 2048;

static byte buffer[ MAX_AVI_BUFFER ];
static int  bufIndex;

/*
===============
Client::safeFS_Write
===============
*/
ID_INLINE void Client::safeFS_Write( const void *buf, int len, fileHandle_t f )
{
  if( FileSystem::write( buf, len, f ) < len )
    Common::error( ERR_DROP, "Failed to write avi file" );
}

/*
===============
Client::wRITE_STRING
===============
*/
ID_INLINE void Client::wRITE_STRING( const char *s )
{
  Com_Memcpy( &buffer[ bufIndex ], s, strlen( s ) );
  bufIndex += strlen( s );
}

/*
===============
Client::wRITE_4BYTES
===============
*/
ID_INLINE void Client::wRITE_4BYTES( int x )
{
  buffer[ bufIndex + 0 ] = static_cast<byte>( ( x >>  0 ) & 0xFF );
  buffer[ bufIndex + 1 ] = static_cast<byte>( ( x >>  8 ) & 0xFF );
  buffer[ bufIndex + 2 ] = static_cast<byte>( ( x >> 16 ) & 0xFF );
  buffer[ bufIndex + 3 ] = static_cast<byte>( ( x >> 24 ) & 0xFF );
  bufIndex += 4;
}

/*
===============
Client::wRITE_2BYTES
===============
*/
ID_INLINE void Client::wRITE_2BYTES( int x )
{
  buffer[ bufIndex + 0 ] = static_cast<byte>( ( x >>  0 ) & 0xFF );
  buffer[ bufIndex + 1 ] = static_cast<byte>( ( x >>  8 ) & 0xFF );
  bufIndex += 2;
}

/*
===============
Client::sTART_CHUNK
===============
*/
ID_INLINE void Client::sTART_CHUNK( const char *s )
{
  if( afd.chunkStackTop == MAX_RIFF_CHUNKS )
  {
    Common::error( ERR_DROP, "ERROR: Top of chunkstack breached" );
  }

  afd.chunkStack[ afd.chunkStackTop ] = bufIndex;
  afd.chunkStackTop++;
  Client::wRITE_STRING( s );
  Client::wRITE_4BYTES( 0 );
}

/*
===============
Client::eND_CHUNK
===============
*/
ID_INLINE void Client::eND_CHUNK( )
{
  int endIndex = bufIndex;

  if( afd.chunkStackTop <= 0 )
  {
    Common::error( ERR_DROP, "ERROR: Bottom of chunkstack breached" );
  }

  afd.chunkStackTop--;
  bufIndex = afd.chunkStack[ afd.chunkStackTop ];
  bufIndex += 4;
  Client::wRITE_4BYTES( endIndex - bufIndex - 4 );
  bufIndex = endIndex;
  bufIndex = PAD( bufIndex, 2 );
}

/*
===============
CL_WriteAVIHeader
===============
*/
static void CL_WriteAVIHeader( )
{
  bufIndex = 0;
  afd.chunkStackTop = 0;

  Client::sTART_CHUNK( "RIFF" );
  {
    Client::wRITE_STRING( "AVI " );
    {
      Client::sTART_CHUNK( "LIST" );
      {
        Client::wRITE_STRING( "hdrl" );
        Client::wRITE_STRING( "avih" );
        Client::wRITE_4BYTES( 56 );                     //"avih" "chunk" size
        Client::wRITE_4BYTES( afd.framePeriod );        //dwMicroSecPerFrame
        Client::wRITE_4BYTES( afd.maxRecordSize *
            afd.frameRate );                    //dwMaxBytesPerSec
        Client::wRITE_4BYTES( 0 );                      //dwReserved1
        Client::wRITE_4BYTES( 0x110 );                  //dwFlags bits HAS_INDEX and IS_INTERLEAVED
        Client::wRITE_4BYTES( afd.numVideoFrames );     //dwTotalFrames
        Client::wRITE_4BYTES( 0 );                      //dwInitialFrame

        if( afd.audio )                         //dwStreams
          Client::wRITE_4BYTES( 2 );
        else
          Client::wRITE_4BYTES( 1 );

        Client::wRITE_4BYTES( afd.maxRecordSize );      //dwSuggestedBufferSize
        Client::wRITE_4BYTES( afd.width );              //dwWidth
        Client::wRITE_4BYTES( afd.height );             //dwHeight
        Client::wRITE_4BYTES( 0 );                      //dwReserved[ 0 ]
        Client::wRITE_4BYTES( 0 );                      //dwReserved[ 1 ]
        Client::wRITE_4BYTES( 0 );                      //dwReserved[ 2 ]
        Client::wRITE_4BYTES( 0 );                      //dwReserved[ 3 ]

        Client::sTART_CHUNK( "LIST" );
        {
          Client::wRITE_STRING( "strl" );
          Client::wRITE_STRING( "strh" );
          Client::wRITE_4BYTES( 56 );                   //"strh" "chunk" size
          Client::wRITE_STRING( "vids" );

          if( afd.motionJpeg )
            Client::wRITE_STRING( "MJPG" );
          else
            Client::wRITE_4BYTES( 0 );                  // BI_RGB

          Client::wRITE_4BYTES( 0 );                    //dwFlags
          Client::wRITE_4BYTES( 0 );                    //dwPriority
          Client::wRITE_4BYTES( 0 );                    //dwInitialFrame

          Client::wRITE_4BYTES( 1 );                    //dwTimescale
          Client::wRITE_4BYTES( afd.frameRate );        //dwDataRate
          Client::wRITE_4BYTES( 0 );                    //dwStartTime
          Client::wRITE_4BYTES( afd.numVideoFrames );   //dwDataLength

          Client::wRITE_4BYTES( afd.maxRecordSize );    //dwSuggestedBufferSize
          Client::wRITE_4BYTES( -1 );                   //dwQuality
          Client::wRITE_4BYTES( 0 );                    //dwSampleSize
          Client::wRITE_2BYTES( 0 );                    //rcFrame
          Client::wRITE_2BYTES( 0 );                    //rcFrame
          Client::wRITE_2BYTES( afd.width );            //rcFrame
          Client::wRITE_2BYTES( afd.height );           //rcFrame

          Client::wRITE_STRING( "strf" );
          Client::wRITE_4BYTES( 40 );                   //"strf" "chunk" size
          Client::wRITE_4BYTES( 40 );                   //biSize
          Client::wRITE_4BYTES( afd.width );            //biWidth
          Client::wRITE_4BYTES( afd.height );           //biHeight
          Client::wRITE_2BYTES( 1 );                    //biPlanes
          Client::wRITE_2BYTES( 24 );                   //biBitCount

          if( afd.motionJpeg )                  //biCompression
          {
            Client::wRITE_STRING( "MJPG" );
            Client::wRITE_4BYTES( afd.width *
                afd.height );                   //biSizeImage
          }
          else
          {
            Client::wRITE_4BYTES( 0 );                  // BI_RGB
            Client::wRITE_4BYTES( afd.width *
                afd.height * 3 );               //biSizeImage
          }

          Client::wRITE_4BYTES( 0 );                    //biXPelsPetMeter
          Client::wRITE_4BYTES( 0 );                    //biYPelsPetMeter
          Client::wRITE_4BYTES( 0 );                    //biClrUsed
          Client::wRITE_4BYTES( 0 );                    //biClrImportant
        }
        Client::eND_CHUNK( );

        if( afd.audio )
        {
          Client::sTART_CHUNK( "LIST" );
          {
            Client::wRITE_STRING( "strl" );
            Client::wRITE_STRING( "strh" );
            Client::wRITE_4BYTES( 56 );                 //"strh" "chunk" size
            Client::wRITE_STRING( "auds" );
            Client::wRITE_4BYTES( 0 );                  //FCC
            Client::wRITE_4BYTES( 0 );                  //dwFlags
            Client::wRITE_4BYTES( 0 );                  //dwPriority
            Client::wRITE_4BYTES( 0 );                  //dwInitialFrame

            Client::wRITE_4BYTES( afd.a.sampleSize );   //dwTimescale
            Client::wRITE_4BYTES( afd.a.sampleSize *
                afd.a.rate );                   //dwDataRate
            Client::wRITE_4BYTES( 0 );                  //dwStartTime
            Client::wRITE_4BYTES( afd.a.totalBytes /
                afd.a.sampleSize );             //dwDataLength

            Client::wRITE_4BYTES( 0 );                  //dwSuggestedBufferSize
            Client::wRITE_4BYTES( -1 );                 //dwQuality
            Client::wRITE_4BYTES( afd.a.sampleSize );   //dwSampleSize
            Client::wRITE_2BYTES( 0 );                  //rcFrame
            Client::wRITE_2BYTES( 0 );                  //rcFrame
            Client::wRITE_2BYTES( 0 );                  //rcFrame
            Client::wRITE_2BYTES( 0 );                  //rcFrame

            Client::wRITE_STRING( "strf" );
            Client::wRITE_4BYTES( 18 );                 //"strf" "chunk" size
            Client::wRITE_2BYTES( afd.a.format );       //wFormatTag
            Client::wRITE_2BYTES( afd.a.channels );     //nChannels
            Client::wRITE_4BYTES( afd.a.rate );         //nSamplesPerSec
            Client::wRITE_4BYTES( afd.a.sampleSize *
                afd.a.rate );                   //nAvgBytesPerSec
            Client::wRITE_2BYTES( afd.a.sampleSize );   //nBlockAlign
            Client::wRITE_2BYTES( afd.a.bits );         //wBitsPerSample
            Client::wRITE_2BYTES( 0 );                  //cbSize
          }
          Client::eND_CHUNK( );
        }
      }
      Client::eND_CHUNK( );

      afd.moviOffset = bufIndex;

      Client::sTART_CHUNK( "LIST" );
      {
        Client::wRITE_STRING( "movi" );
      }
    }
  }
}

/*
===============
Client::openAVIForWriting

Creates an AVI file and gets it into a state where
writing the actual data can begin
===============
*/
bool Client::openAVIForWriting( const char *fileName )
{
  if( afd.fileOpen )
    return false;

  Com_Memset( &afd, 0, sizeof( aviFileData_t ) );

  // Don't start if a framerate has not been chosen
  if( cl_aviFrameRate->asInt() <= 0 )
  {
    Common::printf( S_COLOR_RED "cl_aviFrameRate must be >= 1\n" );
    return false;
  }

  if( ( afd.f = FileSystem::fOpenFileWrite_HomeData( fileName ) ) <= 0 )
    return false;

  if( ( afd.idxF = FileSystem::fOpenFileWrite_HomeData(
          va( "%s" INDEX_FILE_EXTENSION, fileName ) ) ) <= 0 )
  {
    FileSystem::fCloseFile( afd.f );
    return false;
  }

  Q_strncpyz( afd.fileName, fileName, MAX_QPATH );

  afd.frameRate = cl_aviFrameRate->asInt();
  afd.framePeriod = static_cast<int>( 1000000.0f / afd.frameRate );
  afd.width = cls.glconfig.vidWidth;
  afd.height = cls.glconfig.vidHeight;

  if( cl_aviMotionJpeg->asInt() )
    afd.motionJpeg = true;
  else
    afd.motionJpeg = false;

  // Capture buffer stores RGB pixels but OpenGL ES reads RGBA and converts to RGB in-place.
  // Encode buffer only needs to store RGB pixels.
  // Allocate a bit more space for the capture buffer to account for possible
  // padding at the end of pixel lines, and padding for alignment
  #define MAX_PACK_LEN 16
  afd.cBuffer = static_cast<byte *>(Zone::malloc((afd.width * 4 + MAX_PACK_LEN - 1) * afd.height + MAX_PACK_LEN - 1));
  // raw avi files have pixel lines start on 4-byte boundaries
  afd.eBuffer = static_cast<byte *>(Zone::malloc(PAD(afd.width * 3, AVI_LINE_PADDING) * afd.height));

  afd.a.rate = dma.speed;
  afd.a.format = WAV_FORMAT_PCM;
  afd.a.channels = dma.channels;
  /* !!! FIXME: if Client::writeAVIAudioFrame() is ever called from somewhere other
     !!! FIXME:  than S_TransferStereo16(), we will need to handle/convert
     !!! FIXME:  float32 samples for AVI writing. */
  afd.a.bits = dma.samplebits;
  afd.a.sampleSize = ( afd.a.bits / 8 ) * afd.a.channels;

  if( afd.a.rate % afd.frameRate )
  {
    int suggestRate = afd.frameRate;

    while( ( afd.a.rate % suggestRate ) && suggestRate >= 1 )
      suggestRate--;

    Common::printf( S_COLOR_YELLOW "WARNING: cl_aviFrameRate is not a divisor "
        "of the audio rate, suggest %d\n", suggestRate );
  }

  if( !CvarSystem::variableIntegerValue( "s_initsound" ) )
  {
    afd.audio = false;
  }
  else if( Q_stricmp( CvarSystem::variableString( "s_backend" ), "OpenAL" ) )
  {
    if( afd.a.bits != 16 || afd.a.channels != 2 )
    {
      Common::printf( S_COLOR_YELLOW "WARNING: Audio format of %d bit/%d channels not supported",
          afd.a.bits, afd.a.channels );
      afd.audio = false;
    }
    else
      afd.audio = true;
  }
  else
  {
    afd.audio = false;
    Common::printf( S_COLOR_YELLOW "WARNING: Audio capture is not supported "
        "with OpenAL. Set s_useOpenAL to 0 for audio capture\n" );
  }

  // This doesn't write a real header, but allocates the
  // correct amount of space at the beginning of the file
  CL_WriteAVIHeader( );

  Client::safeFS_Write( buffer, bufIndex, afd.f );
  afd.fileSize = bufIndex;

  bufIndex = 0;
  Client::sTART_CHUNK( "idx1" );
  Client::safeFS_Write( buffer, bufIndex, afd.idxF );

  afd.moviSize = 4; // For the "movi"
  afd.fileOpen = true;

  return true;
}

/*
===============
Client::checkFileSize
===============
*/
bool Client::checkFileSize( int bytesToAdd )
{
  unsigned int newFileSize;

  newFileSize =
    afd.fileSize +                // Current file size
    bytesToAdd +                  // What we want to add
    ( afd.numIndices * 16 ) +     // The index
    4;                            // The index size

  // I assume all the operating systems
  // we target can handle a 2Gb file
  if( newFileSize > INT_MAX )
  {
    // Close the current file...
    Client::closeAVI( );

    // ...And open a new one
    Client::openAVIForWriting( va( "%s_", afd.fileName ) );

    return true;
  }

  return false;
}

/*
===============
Client::writeAVIVideoFrame
===============
*/
void Client::writeAVIVideoFrame( const byte *imageBuffer, int size )
{
  int   chunkOffset = afd.fileSize - afd.moviOffset - 8;
  int   chunkSize = 8 + size;
  int   paddingSize = PADLEN(size, 2);
  byte  padding[ 4 ] = { 0 };

  if( !afd.fileOpen )
    return;

  // Chunk header + contents + padding
  if( Client::checkFileSize( 8 + size + 2 ) )
    return;

  bufIndex = 0;
  Client::wRITE_STRING( "00dc" );
  Client::wRITE_4BYTES( size );

  Client::safeFS_Write( buffer, 8, afd.f );
  Client::safeFS_Write( imageBuffer, size, afd.f );
  Client::safeFS_Write( padding, paddingSize, afd.f );
  afd.fileSize += ( chunkSize + paddingSize );

  afd.numVideoFrames++;
  afd.moviSize += ( chunkSize + paddingSize );

  if( size > afd.maxRecordSize )
    afd.maxRecordSize = size;

  // Index
  bufIndex = 0;
  Client::wRITE_STRING( "00dc" );           //dwIdentifier
  Client::wRITE_4BYTES( 0x00000010 );       //dwFlags (all frames are KeyFrames)
  Client::wRITE_4BYTES( chunkOffset );      //dwOffset
  Client::wRITE_4BYTES( size );             //dwLength
  Client::safeFS_Write( buffer, 16, afd.idxF );

  afd.numIndices++;
}

constexpr int PCM_BUFFER_SIZE = 44100;

/*
===============
Client::writeAVIAudioFrame
===============
*/
void Client::writeAVIAudioFrame( const byte *pcmBuffer, int size )
{
  static byte pcmCaptureBuffer[ PCM_BUFFER_SIZE ] = { 0 };
  static int  bytesInBuffer = 0;

  if( !afd.audio )
    return;

  if( !afd.fileOpen )
    return;

  // Chunk header + contents + padding
  if( Client::checkFileSize( 8 + bytesInBuffer + size + 2 ) )
    return;

  if( bytesInBuffer + size > PCM_BUFFER_SIZE )
  {
    Common::printf( S_COLOR_YELLOW
        "WARNING: Audio capture buffer overflow -- truncating\n" );
    size = PCM_BUFFER_SIZE - bytesInBuffer;
  }

  Com_Memcpy( &pcmCaptureBuffer[ bytesInBuffer ], pcmBuffer, size );
  bytesInBuffer += size;

  // Only write if we have a frame's worth of audio
  if( bytesInBuffer >= static_cast<int>(ceil( static_cast<float>(afd.a.rate) / static_cast<float>(afd.frameRate) )) *
        afd.a.sampleSize )
  {
    int   chunkOffset = afd.fileSize - afd.moviOffset - 8;
    int   chunkSize = 8 + bytesInBuffer;
    int   paddingSize = PADLEN(bytesInBuffer, 2);
    byte  padding[ 4 ] = { 0 };

    bufIndex = 0;
    Client::wRITE_STRING( "01wb" );
    Client::wRITE_4BYTES( bytesInBuffer );

    Client::safeFS_Write( buffer, 8, afd.f );
    Client::safeFS_Write( pcmCaptureBuffer, bytesInBuffer, afd.f );
    Client::safeFS_Write( padding, paddingSize, afd.f );
    afd.fileSize += ( chunkSize + paddingSize );

    afd.numAudioFrames++;
    afd.moviSize += ( chunkSize + paddingSize );
    afd.a.totalBytes += bytesInBuffer;

    // Index
    bufIndex = 0;
    Client::wRITE_STRING( "01wb" );           //dwIdentifier
    Client::wRITE_4BYTES( 0 );                //dwFlags
    Client::wRITE_4BYTES( chunkOffset );      //dwOffset
    Client::wRITE_4BYTES( bytesInBuffer );    //dwLength
    Client::safeFS_Write( buffer, 16, afd.idxF );

    afd.numIndices++;

    bytesInBuffer = 0;
  }
}

/*
===============
Client::takeVideoFrame
===============
*/
void Client::takeVideoFrame( )
{
  // AVI file isn't open
  if( !afd.fileOpen )
    return;

  refExport.TakeVideoFrame( afd.width, afd.height,
      afd.cBuffer, afd.eBuffer, afd.motionJpeg );
}

/*
===============
Client::closeAVI

Closes the AVI file and writes an index chunk
===============
*/
bool Client::closeAVI( )
{
  int indexRemainder;
  int indexSize = afd.numIndices * 16;
  const char *idxFileName = va( "%s" INDEX_FILE_EXTENSION, afd.fileName );

  // AVI file isn't open
  if( !afd.fileOpen )
    return false;

  afd.fileOpen = false;

  FileSystem::seek( afd.idxF, 4, FS_SEEK_SET );
  bufIndex = 0;
  Client::wRITE_4BYTES( indexSize );
  Client::safeFS_Write( buffer, bufIndex, afd.idxF );
  FileSystem::fCloseFile( afd.idxF );

  // Write index

  // Open the temp index file
  if( ( indexSize = FileSystem::fOpenFileRead( idxFileName,
          &afd.idxF, true ) ) <= 0 )
  {
    FileSystem::fCloseFile( afd.f );
    return false;
  }

  indexRemainder = indexSize;

  // Append index to end of avi file
  while( indexRemainder > MAX_AVI_BUFFER )
  {
    FileSystem::read( buffer, MAX_AVI_BUFFER, afd.idxF );
    Client::safeFS_Write( buffer, MAX_AVI_BUFFER, afd.f );
    afd.fileSize += MAX_AVI_BUFFER;
    indexRemainder -= MAX_AVI_BUFFER;
  }
  FileSystem::read( buffer, indexRemainder, afd.idxF );
  Client::safeFS_Write( buffer, indexRemainder, afd.f );
  afd.fileSize += indexRemainder;
  FileSystem::fCloseFile( afd.idxF );

  // Remove temp index file
  FileSystem::remove_HomeData( idxFileName );

  // Write the real header
  FileSystem::seek( afd.f, 0, FS_SEEK_SET );
  CL_WriteAVIHeader( );

  bufIndex = 4;
  Client::wRITE_4BYTES( afd.fileSize - 8 ); // "RIFF" size

  bufIndex = afd.moviOffset + 4;    // Skip "LIST"
  Client::wRITE_4BYTES( afd.moviSize );

  Client::safeFS_Write( buffer, bufIndex, afd.f );

  Zone::free( afd.cBuffer );
  Zone::free( afd.eBuffer );
  FileSystem::fCloseFile( afd.f );

  Common::printf( "Wrote %d:%d frames to %s\n", afd.numVideoFrames, afd.numAudioFrames, afd.fileName );

  return true;
}

/*
===============
Client::videoRecording
===============
*/
bool Client::videoRecording( )
{
  return afd.fileOpen;
}
