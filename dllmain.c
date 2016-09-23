#include "dll.h"
#include "ftrAnsiSdk.h"


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32_WCE
#include <sys/types.h>
#include <sys/stat.h>
#endif

#if defined (FTR_OS_DOS_WIN_OS2)
#pragma warning(disable : 4996)
#else
#include <sys/time.h>
#endif

#ifndef _WIN32_WCE
#include <fcntl.h>
#endif

#if defined(__WIN32__)
#include <windows.h>
#pragma pack(push, 1)
#endif

/* Defined in ftrScanApi.h */
#if defined(FTR_OS_UNIX)
#include <dirent.h>

typedef struct __attribute__ ((aligned(1),packed)) tagBITMAPINFOHEADER{
    FTR_UINT32   biSize;
    FTR_INT32    biWidth;
    FTR_INT32    biHeight;
    FTR_UINT16   biPlanes;
    FTR_UINT16   biBitCount;
    FTR_UINT32   biCompression;
    FTR_UINT32   biSizeImage;
    FTR_INT32    biXPelsPerMeter;
    FTR_INT32    biYPelsPerMeter;
    FTR_UINT32   biClrUsed;
    FTR_UINT32   biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct __attribute__ ((aligned(1),packed)) tagRGBQUAD {
    FTR_UINT8   rgbBlue;
    FTR_UINT8   rgbGreen;
    FTR_UINT8   rgbRed;
    FTR_UINT8   rgbReserved;
} RGBQUAD;

typedef struct __attribute__ ((aligned(1),packed)) tagBITMAPINFO {
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[1];
} BITMAPINFO, *PBITMAPINFO;

typedef struct __attribute__ ((aligned(1),packed)) tagBITMAPFILEHEADER {
    FTR_UINT16   bfType;
    FTR_UINT32   bfSize;
    FTR_UINT16   bfReserved1;
    FTR_UINT16   bfReserved2;
    FTR_UINT32   bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

#endif

#if defined(__WIN32__)
#pragma pack(pop)
#endif

#define CHECK_ARGV1_PARAM( s ) if( 0 == strcmp( s, argv[ 1 ] ) )
char stError[128];

typedef char FILENAME[128];
char* tempExt[] = { ".ansi", ".iso" };

DLLIMPORT void PrintErrorMessage( unsigned long nErrCode )
{
    printf("Operation failed. ");    
    switch( nErrCode ) 
    {
    case 0:
        strcpy( stError, "OK" );
        break;
    case FTR_ERROR_EMPTY_FRAME:
        strcpy( stError, "- Empty frame -" );
        break;
    case FTR_ERROR_MOVABLE_FINGER:
        strcpy( stError, "- Movable finger -" );
        break;
    case FTR_ERROR_NO_FRAME:
        strcpy( stError, "- Fake finger -" );
        break;
    case FTR_ERROR_HARDWARE_INCOMPATIBLE:
        strcpy( stError, "- Incompatible hardware -" );
        break;
    case FTR_ERROR_FIRMWARE_INCOMPATIBLE:
        strcpy( stError, "- Incompatible firmware -" );
        break;
    case FTR_ERROR_INVALID_AUTHORIZATION_CODE:
        strcpy( stError, "- Invalid authorization code -" );
        break;
    case FTR_ANSISDK_ERROR_IMAGE_SIZE_NOT_SUP:
        strcpy( stError, "- Image size is not supported -" );
        break;
    case FTR_ANSISDK_ERROR_EXTRACTION_UNSPEC:
        strcpy( stError, "- Unspecified extraction error -" );
        break;
    case FTR_ANSISDK_ERROR_EXTRACTION_BAD_IMP:
        strcpy( stError, "- Incorrect impression type -" );
        break;
    case FTR_ANSISDK_ERROR_MATCH_NULL:
    case FTR_ANSISDK_ERROR_MATCH_PARSE_PROBE:
    case FTR_ANSISDK_ERROR_MATCH_PARSE_GALLERY:
        strcpy( stError, "- Incorrect parameter -" );
        break;
    default:
        sprintf( stError, "Unknown return code - %lu", nErrCode );
    }

    printf( "%s\n", stError );
}




int SaveBmpFile( char* file_name, unsigned char *pImage, int width, int height )
{
    BITMAPINFO *pDIBHeader;
    BITMAPFILEHEADER  bmfHeader;
    int iCyc;
    FILE* fp;

    unsigned char *cptrData;
    unsigned char *cptrDIBData;
    unsigned char *pDIBData;

    /* Allocate memory for a DIB header */
    if( (pDIBHeader = (BITMAPINFO *)malloc( sizeof( BITMAPINFO ) + sizeof( RGBQUAD ) * 255 )) == NULL )
    {
        printf( "SaveBmpFile: memory allocation error!\n" );
        return -1;
    }
    memset( (void *)pDIBHeader, 0, sizeof( BITMAPINFO ) + sizeof( RGBQUAD ) * 255 );
    
    /* Fill the DIB header */
    pDIBHeader->bmiHeader.biSize          = sizeof( BITMAPINFOHEADER );
    pDIBHeader->bmiHeader.biWidth         = width;
    pDIBHeader->bmiHeader.biHeight        = height;
    pDIBHeader->bmiHeader.biPlanes        = 1;
    pDIBHeader->bmiHeader.biBitCount      = 8;      /* 8bits gray scale bmp */
    pDIBHeader->bmiHeader.biCompression   = 0;
    
    /* Initialize logical and DIB gray scale */
    for( iCyc = 0; iCyc < 256; iCyc++ )
    {
        pDIBHeader->bmiColors[iCyc].rgbBlue = pDIBHeader->bmiColors[iCyc].rgbGreen = pDIBHeader->bmiColors[iCyc].rgbRed = (unsigned char)iCyc;
    }

    bmfHeader.bfType = 0x42 + 0x4D * 0x100;
    bmfHeader.bfSize = 14 + sizeof( BITMAPINFO ) + sizeof( RGBQUAD ) * 255 + width * height;
    bmfHeader.bfOffBits = 14 + pDIBHeader->bmiHeader.biSize + sizeof( RGBQUAD ) * 256;

    /* Write to file */
    fp = fopen( file_name, "wb" );
    if( fp == NULL )
    {
        printf( "SaveBmpFile: can not create picture file!\n" );
        free( pDIBHeader );
        return 0;
    }

    fwrite( (void *)&bmfHeader.bfType, sizeof(FTR_UINT16), 1, fp );
    fwrite( (void *)&bmfHeader.bfSize, sizeof(FTR_UINT32), 1, fp );
    fwrite( (void *)&bmfHeader.bfReserved1, sizeof(FTR_UINT16), 1, fp );
    fwrite( (void *)&bmfHeader.bfReserved2, sizeof(FTR_UINT16), 1, fp );
    fwrite( (void *)&bmfHeader.bfOffBits, sizeof(FTR_UINT32), 1, fp );

    fwrite( (void *)&pDIBHeader->bmiHeader.biSize, sizeof(FTR_UINT32), 1, fp);
    fwrite( (void *)&pDIBHeader->bmiHeader.biWidth, sizeof(FTR_INT32), 1, fp);
    fwrite( (void *)&pDIBHeader->bmiHeader.biHeight, sizeof(FTR_INT32), 1, fp);
    fwrite( (void *)&pDIBHeader->bmiHeader.biPlanes, sizeof(FTR_UINT16), 1, fp);
    fwrite( (void *)&pDIBHeader->bmiHeader.biBitCount, sizeof(FTR_UINT16), 1, fp);
    fwrite( (void *)&pDIBHeader->bmiHeader.biCompression, sizeof(FTR_UINT32), 1, fp);
    fwrite( (void *)&pDIBHeader->bmiHeader.biSizeImage, sizeof(FTR_UINT32), 1, fp);
    fwrite( (void *)&pDIBHeader->bmiHeader.biXPelsPerMeter, sizeof(FTR_INT32), 1, fp);
    fwrite( (void *)&pDIBHeader->bmiHeader.biYPelsPerMeter, sizeof(FTR_INT32), 1, fp);
    fwrite( (void *)&pDIBHeader->bmiHeader.biClrUsed, sizeof(FTR_UINT32), 1, fp);
    fwrite( (void *)&pDIBHeader->bmiHeader.biClrImportant, sizeof(FTR_UINT32), 1, fp);

    for( iCyc=0 ; iCyc<256 ; iCyc++ )
    {
        fwrite( (void *)&pDIBHeader->bmiColors[iCyc].rgbBlue, sizeof(FTR_UINT8), 1, fp );
        fwrite( (void *)&pDIBHeader->bmiColors[iCyc].rgbGreen, sizeof(FTR_UINT8), 1, fp );
        fwrite( (void *)&pDIBHeader->bmiColors[iCyc].rgbRed, sizeof(FTR_UINT8), 1, fp );
        fwrite( (void *)&pDIBHeader->bmiColors[iCyc].rgbReserved, sizeof(FTR_UINT8), 1, fp );
    }
    
    /* Copy fingerprint image */
    pDIBData = (unsigned char*)malloc( height * width);
    memset( (void *)pDIBData, 0, height * width );

    cptrData = pImage + (height - 1) * width;
    cptrDIBData = pDIBData;
    
    for( iCyc = 0; iCyc < height; iCyc++ )
    {
        memcpy( cptrDIBData, cptrData, width );
        cptrData = cptrData - width;
        cptrDIBData = cptrDIBData + width;
    }

    fwrite( (void *)pDIBData, 1, width * height, fp );
    fclose( fp );
    free( pDIBData );
    free( pDIBHeader );
    return 1;
}




int ReadTemplateFile( char* file_name, unsigned char *buffer, int size )
{
    FILE* fp = NULL;
    size_t file_size = 0;
    int read_size = 0;

    fp = fopen( file_name, "r" );
    if( fp == NULL )
    {
        printf( "ReadTemplateFile: can not read file %s!\n", file_name );
        return 0;
    }

    fseek( fp, 0L, SEEK_END );
    file_size = ftell( fp );
    fseek( fp, 0L, SEEK_SET );

    read_size = min( (size_t)size, file_size );
    fread( buffer, read_size, 1, fp );

    fclose( fp );

    return read_size;
}


int SaveTemplateFile( char* file_name, unsigned char *buffer, int size )
{
    FILE* fp;
    fp = fopen( file_name, "wb" );
    if( fp == NULL )
    {
        printf( "SaveTemplateFile: can not create file %s!\n", file_name );
        return 0;
    }

    fwrite( (void *)buffer, 1, size, fp );
    fclose( fp );

    return 1;
}

long GetTimeMS()
{
    long lRet = 0;

#if defined(FTR_OS_UNIX)
    struct timeval tv;

    gettimeofday( &tv , NULL );
    lRet = tv.tv_sec * 1000 + tv.tv_usec / 1000;
#else
    FILETIME ft;
    unsigned long long tmpres;
    long tv_sec;
    long tv_usec;

#if defined(_WIN32_WCE)
    SYSTEMTIME sys_time;
    GetSystemTime( &sys_time );
    SystemTimeToFileTime(&sys_time,&ft);
#else
    GetSystemTimeAsFileTime( &ft );
#endif
    tmpres = ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;

    /* Converting file time to unix epoch */
    tmpres -= 11644473600000000ULL; 

    tmpres /= 10;  /* Convert into microseconds */

    tv_sec = (long)(tmpres / 1000000UL);
    tv_usec = (long)(tmpres % 1000000UL);

    lRet = tv_sec * 1000 + tv_usec / 1000;
#endif

    return lRet;
}


void SimulateDelay()
{
    long start_time = GetTimeMS();

    while( GetTimeMS() - start_time < 100 );
}

FILENAME* GetDirectoryFileList( char* dir, int* fileCount )
{
#if defined(FTR_OS_UNIX)
    DIR *dp;
    struct dirent *ep;
#else
    char dirFileName[256];
    HANDLE   hFind;
    WIN32_FIND_DATA   ff;
#endif
    
    int tmpl_count = 0;
    int curItemCount = 50;
    FILENAME* res = (FILENAME*)malloc(sizeof(FILENAME)*curItemCount);

    if(!res)
    {
        return NULL;
    }

    *fileCount = 0;

#if defined(FTR_OS_UNIX)
    for(tmpl_count = 0; tmpl_count < 2; tmpl_count++)
    {
        dp = opendir(dir);

        if(!dp)
        {
            free(res);
            return NULL;
        }

        while (ep = readdir (dp))
        {
            if( strstr( ep->d_name, tempExt[tmpl_count] ) != NULL )
            {
                strcpy( res[*fileCount], ep->d_name ); 
                ( *fileCount )++;

                if(*fileCount == curItemCount)
                {
                    FILENAME* new_res = (FILENAME*)malloc( sizeof(FILENAME)*curItemCount*2 );
                    if(!new_res)
                    {
                        free(res);
                        return NULL;
                    }

                    memcpy(new_res,res,sizeof(FILENAME)*curItemCount);
                    free(res);
                    res = new_res;
                    curItemCount = curItemCount * 2;
                }
            }
        }

        closedir(dp);
    }
#else
    for(tmpl_count = 0; tmpl_count < 2; tmpl_count++)
    {
    
        strcpy( dirFileName, dir );
        strcat( dirFileName, "\\*" );
        strcat( dirFileName, tempExt[tmpl_count]);

        hFind = FindFirstFile( dirFileName, &ff );

        if(!hFind)
        {
            free(res);
            return NULL;
        }

        do
        {
            if (ff.dwFileAttributes & ~FILE_ATTRIBUTE_DIRECTORY)
            {
                strcpy( res[*fileCount], ff.cFileName ); 
                ( *fileCount )++;

                if(*fileCount == curItemCount)
                {
                    FILENAME* new_res = (FILENAME*)malloc( sizeof(FILENAME)*curItemCount*2 );
                    if(!new_res)
                    {
                        free(res);
                        return NULL;
                    }

                    memcpy(new_res,res,sizeof(FILENAME)*curItemCount);
                    free(res);
                    res = new_res;
                    curItemCount = curItemCount * 2;
                }
            }
        }
        while (FindNextFile(hFind, &ff) != 0);

        FindClose(hFind);
    }
#endif
    return res;
}

//int CaprureImage( FTRHANDLE dev, char* save_file_name, PFTRSCAN_IMAGE_SIZE img_size )
//{
//    int res = 0;
//    int i = 0;
//    FTR_BYTE* picture_buffer = NULL;
//
//    picture_buffer = (FTR_BYTE*)malloc( img_size->nImageSize );
//
//    if( !picture_buffer )
//    {
//        printf( "CaprureImage: memory allocation error!\n" );
//        return res;
//    }
//
//	printf( "CaprureImage: Put finger to scanner\n" );
//    
//    for( ; ; )
//    {
//        long start_time = GetTimeMS();
//        FTR_BOOL ret = ftrAnsiSdkCaptureImage( dev, picture_buffer );
//
//        if( ret )
//        {
//            printf( "Capture image done. Time: %lu ms\n", GetTimeMS() - start_time ); 
//
//            if( SaveBmpFile( save_file_name, picture_buffer, img_size->nWidth, img_size->nHeight ) )
//            {
//                 printf( "Save image to %s done\n", save_file_name );
//                 res = 1;
//            }
//            else
//            {
//                printf( "CaprureImage: failed to save image to file\n" );
//            }
//            break;
//        }
//        else
//        {
//            FTR_DWORD dwError = ftrScanGetLastError();
//            if( dwError != FTR_ERROR_EMPTY_FRAME && dwError != FTR_ERROR_NO_FRAME && dwError != FTR_ERROR_MOVABLE_FINGER )
//            {
//                PrintErrorMessage( dwError );
//                break;
//            }
//        }
//
//        SimulateDelay();
//    }
//    
//    free( picture_buffer );
//    return res;
//}
//
//int CreateTemplate( FTRHANDLE dev, char* save_file_name, PFTRSCAN_IMAGE_SIZE img_size )
//{
//    int res = 0;
//    int i = 0;
//    FTR_BYTE* picture_buffer = NULL;
//    FTR_BYTE* template_buffer = NULL;
//
//    picture_buffer = (FTR_BYTE*)malloc( img_size->nImageSize );
//
//    if( !picture_buffer )
//    {
//        printf( "CreateTemplate: memory allocation error!\n" );
//        return res;
//    }
//
//    template_buffer = (FTR_BYTE*)malloc( ftrAnsiSdkGetMaxTemplateSize() );
//
//    if( !template_buffer )
//    {
//        printf( "CreateTemplate: memory allocation error!\n" );
//        free( picture_buffer );
//        return res;
//    }
//
//	printf( "CreateTemplate: Put finger to scanner\n" );
//    
//    for( ; ; )
//    {
//        int template_size = 0;
//        long start_time = GetTimeMS();
//        
//        FTR_BOOL ret = ftrAnsiSdkCreateTemplate( dev, FTR_ANSISDK_FINGPOS_UK, picture_buffer, template_buffer, &template_size );
//
//        if( ret )
//        {
//            printf( "Create template done. Time: %lu ms\n", GetTimeMS() - start_time );
//
//            if( SaveTemplateFile( save_file_name, template_buffer, template_size ) )
//            {
//                printf( "Save template to %s done.\n", save_file_name );
//                res = 1;
//            }
//            else
//            {
//                printf( "CreateTemplate: failed to save template to file\n" );
//            }
//            break;
//        }
//        else
//        {
//            FTR_DWORD dwError = ftrScanGetLastError();
//            if( dwError != FTR_ERROR_EMPTY_FRAME && dwError != FTR_ERROR_NO_FRAME && dwError != FTR_ERROR_MOVABLE_FINGER )
//            {
//                PrintErrorMessage( dwError );
//                break;
//            }
//        }
//
//        SimulateDelay();
//    }
//
//    free( template_buffer );
//    free( picture_buffer );
//    return res;
//}
//
//int VerifyTemplate( FTRHANDLE dev, char* tmpl_file_name, PFTRSCAN_IMAGE_SIZE img_size )
//{
//    int res = 0;
//    int i = 0;
//    FTR_BYTE* picture_buffer = NULL;
//    FTR_BYTE* template_buffer = NULL;
//
//    picture_buffer = (FTR_BYTE*)malloc( img_size->nImageSize );
//
//    if( !picture_buffer )
//    {
//        printf( "VerifyTemplate: memory allocation error!\n" );
//        return res;
//    }
//
//    template_buffer = (FTR_BYTE*)malloc( ftrAnsiSdkGetMaxTemplateSize() );
//
//    if( !template_buffer )
//    {
//        printf( "VerifyTemplate: memory allocation error!\n" );
//        free( picture_buffer );
//        return res;
//    }
//
//    if( ReadTemplateFile( tmpl_file_name, template_buffer, ftrAnsiSdkGetMaxTemplateSize() ) > 0 )
//    {
//        printf( "VerifyTemplate: Put finger to scanner\n" );
//        for( ; ; )
//        {
//            float vrf_res = 0;
//            long start_time = GetTimeMS();
//
//            FTR_BOOL ret = ftrAnsiSdkVerifyTemplate( dev, FTR_ANSISDK_FINGPOS_UK, template_buffer, picture_buffer, &vrf_res );
//
//            if( ret )
//            {
//                printf( "Verify template done. Result: %f. Time: %lu ms\n", vrf_res, GetTimeMS()-start_time );
//                res = 1;
//                break;
//            }
//            else
//            {
//                FTR_DWORD dwError = ftrScanGetLastError();
//                if( dwError != FTR_ERROR_EMPTY_FRAME && dwError != FTR_ERROR_NO_FRAME && dwError != FTR_ERROR_MOVABLE_FINGER )
//                {
//                    PrintErrorMessage( dwError );
//                    break;
//                }
//            }
//
//            SimulateDelay();
//        }
//    }
//    else
//    {
//        printf( "VerifyTemplate: failed to read template\n" );
//    }
//
//    free( template_buffer );
//    free( picture_buffer );
//
//    return res;
//}
//
//int ConvertTemplate( char* ansi_tmpl_file_name, char* iso_tmpl_file_name )
//{
//    int res = 0;
//    FTR_BYTE* ansi_template_buffer = NULL;
//    FTR_BYTE* iso_template_buffer = NULL;
//    int iso_template_length = 0;
//
//    ansi_template_buffer = (FTR_BYTE*)malloc( ftrAnsiSdkGetMaxTemplateSize() );
//    if( !ansi_template_buffer )
//    {
//        printf( "ConvertTemplate: memory allocation error!\n" );
//        return res;
//    }
//
//    if( ReadTemplateFile( ansi_tmpl_file_name, ansi_template_buffer, ftrAnsiSdkGetMaxTemplateSize() ) > 0 )
//    {
//        FTR_DWORD error = 0;
//        FTR_BOOL ftr_res = ftrAnsiSdkConvertAnsiTemplateToIso( ansi_template_buffer, NULL, &iso_template_length );
//        error = ftrScanGetLastError();
//        if( !ftr_res && FTR_ANSISDK_ERROR_MORE_DATA == error )
//        {
//            iso_template_buffer = (FTR_BYTE*)malloc( iso_template_length );
//            if( iso_template_buffer )
//            {
//                if( ftrAnsiSdkConvertAnsiTemplateToIso( ansi_template_buffer, iso_template_buffer, &iso_template_length ) )
//                {
//                    if( SaveTemplateFile( iso_tmpl_file_name, iso_template_buffer, iso_template_length ) )
//                    {
//                        printf( "Convert template to %s done.\n", iso_tmpl_file_name );
//                        res = 1;
//                    }
//                    else
//                    {
//                        printf( "ConvertTemplate: failed to save template to file\n" );
//                    }
//                }
//                else
//                {
//                    error = ftrScanGetLastError();
//                    printf( "ConvertTemplate: failed to convert template. Error: %u\n", (unsigned int)error );
//                }
//            }
//            else
//            {
//                printf( "ConvertTemplate: memory allocation error!\n" );
//            }
//        }
//        else
//        {
//            printf( "ConvertTemplate: failed to check ANSI template. Error: %u\n", (unsigned int)error );
//        }
//    }
//    else
//    {
//        printf( "ConvertTemplate: failed to read template\n" );
//    }
//
//    if( iso_template_buffer )
//    {
//        free( iso_template_buffer );
//    }
//
//    free( ansi_template_buffer );
//
//    return res;
//}
//
//int IdentifyTemplate( FTRHANDLE dev, char* tmpl_dir_name, PFTRSCAN_IMAGE_SIZE img_size )
//{
//    int res = 0;
//    int i = 0;
//    FTR_BYTE* picture_buffer = NULL;
//    FTR_BYTE* template_buffer = NULL;
//    FTR_BYTE* base_template_buffer = NULL;
//
//    picture_buffer = (FTR_BYTE*)malloc( img_size->nImageSize );
//
//    if( !picture_buffer )
//    {
//        printf( "IdentifyTemplate: memory allocation error!\n" );
//        return res;
//    }
//
//    template_buffer = (FTR_BYTE*)malloc( ftrAnsiSdkGetMaxTemplateSize() );
//
//    if( !template_buffer )
//    {
//        printf( "IdentifyTemplate: memory allocation error!\n" );
//        free( picture_buffer );
//        return res;
//    }
//
//    base_template_buffer = (FTR_BYTE*)malloc( ftrAnsiSdkGetMaxTemplateSize() );
//
//    if( !base_template_buffer )
//    {
//        printf( "IdentifyTemplate: memory allocation error!\n" );
//        free( picture_buffer );
//        free(template_buffer);
//        return res;
//    }
//
//    printf( "IdentifyTemplate: Put finger to scanner\n" );
//
//    for(;;)
//    {
//        int template_size = 0;
//        long start_time = GetTimeMS();
//
//        FTR_BOOL ret = ftrAnsiSdkCreateTemplate( dev, FTR_ANSISDK_FINGPOS_UK, picture_buffer, base_template_buffer, &template_size );
//
//        if( ret )
//        {
//            FILENAME* file_list;
//            int file_count = 0;
//            int file_index = 0;
//            int found = 0;
//            
//            printf( "Create template done. Time: %lu ms\n", GetTimeMS() - start_time );
//                        
//            file_list = GetDirectoryFileList(tmpl_dir_name,&file_count);
//
//            if(!file_list)
//            {
//                 printf( "IdentifyTemplate: Can't get directory content\n" );
//                 break;
//            }
//
//            if(0 == file_count)
//            {
//                printf( "IdentifyTemplate: Can't find templates in directory\n" );
//                free(file_list);
//                break;
//            }
//
//            start_time = GetTimeMS();
//            for(file_index = 0; file_index < file_count; file_index++)
//            {
//                char full_file_name[256];
//                strcpy(full_file_name, tmpl_dir_name);
//#if defined(FTR_OS_UNIX)
//                strcat(full_file_name, "/");
//#else
//                strcat(full_file_name, "\\");
//#endif
//                strcat(full_file_name,file_list[file_index]);
//
//                if( ReadTemplateFile( full_file_name, template_buffer, ftrAnsiSdkGetMaxTemplateSize() ) > 0 )
//                {
//                    float mres = 0;
//                    long start_time = GetTimeMS();
//
//                    FTR_BOOL ret = ftrAnsiSdkMatchTemplates( base_template_buffer, template_buffer, &mres );
//
//                    if( ret && mres > FTR_ANSISDK_MATCH_SCORE_MEDIUM )
//                    {
//                        printf( "Template found. Template name: %s. Result: %f. Time: %lu ms\n", file_list[file_index], mres, GetTimeMS()-start_time );
//                        res = 1;
//                        break;
//                    }
//                }
//                else
//                {
//                     printf( "IdentifyTemplate: Failed to read template file %s\n", full_file_name );
//                }
//            }
//                      
//            free(file_list);
//            break;
//        }
//        else
//        {
//            FTR_DWORD dwError = ftrScanGetLastError();
//            if( dwError != FTR_ERROR_EMPTY_FRAME && dwError != FTR_ERROR_NO_FRAME && dwError != FTR_ERROR_MOVABLE_FINGER )
//            {
//                PrintErrorMessage( dwError );
//                break;
//            }
//        }
//
//        SimulateDelay();
//    }
//
//    free(base_template_buffer);
//    free( template_buffer );
//    free( picture_buffer );
//    return res;
//}
//
//int OpenDevice( FTRHANDLE *dev, FTRSCAN_IMAGE_SIZE * image_size )
//{
//    int res = 0;
//    *dev = ftrScanOpenDevice();
//
//    if( *dev == NULL )
//    {
//        printf( "Failed to open device! Error: 0x%lx\n", ftrScanGetLastError() );
//        return res;
//    }
//
//    printf( "Open device done\n" );
//
//    if( !ftrScanGetImageSize( *dev, image_size ) )
//    {
//        printf( "Failed to get image size. Error: 0x%lx\n", ftrScanGetLastError() );
//        ftrScanCloseDevice( *dev );
//        return res;
//    }
//
//    printf( "Image %dx%d size is %d\n", image_size->nWidth, image_size->nHeight, image_size->nImageSize );
//    res = 1;
//
//    return res;
//}



DLLIMPORT void HelloWorld()
{
	MessageBox(0,"Hello World from DLL!\n","Hi",MB_ICONINFORMATION);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	switch(fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			break;
		}
		case DLL_THREAD_ATTACH:
		{
			break;
		}
		case DLL_THREAD_DETACH:
		{
			break;
		}
	}
	
	/* Return TRUE on success, FALSE on failure */
	return TRUE;
}

