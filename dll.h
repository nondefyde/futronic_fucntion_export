#include "ftrAnsiSdk.h"
#ifndef _DLL_H_
#define _DLL_H_

#if BUILDING_DLL
#define DLLIMPORT __declspec(dllexport)
#else
#define DLLIMPORT __declspec(dllimport)
#endif

DLLIMPORT void HelloWorld();

DLLIMPORT void PrintErrorMessage(unsigned long nErrCode);
DLLIMPORT int SaveBmpFile( char* file_name, unsigned char *pImage, int width, int height );
DLLIMPORT int ReadTemplateFile( char* file_name, unsigned char *buffer, int size );
DLLIMPORT int SaveTemplateFile( char* file_name, unsigned char *buffer, int size );
//DLLIMPORT int CaprureImage( FTRHANDLE dev, char* save_file_name, PFTRSCAN_IMAGE_SIZE img_size );
//DLLIMPORT int CreateTemplate( FTRHANDLE dev, char* save_file_name, PFTRSCAN_IMAGE_SIZE img_size );
//DLLIMPORT int VerifyTemplate( FTRHANDLE dev, char* tmpl_file_name, PFTRSCAN_IMAGE_SIZE img_size );
//DLLIMPORT int ConvertTemplate( char* ansi_tmpl_file_name, char* iso_tmpl_file_name );
//DLLIMPORT int IdentifyTemplate( FTRHANDLE dev, char* tmpl_dir_name, PFTRSCAN_IMAGE_SIZE img_size );
//DLLIMPORT int OpenDevice( FTRHANDLE *dev, FTRSCAN_IMAGE_SIZE * image_size );

#endif
