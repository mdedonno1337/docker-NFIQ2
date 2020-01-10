/******************************************************************************
 * 
 * stdint.h
 * 
 * (c) secunet Security Networks AG
 * 
 * Version information
 * $Date: 2011/05/30 $
 * $Change: 111030 $
 * $File: //BSI/NFIQ2/develop/main/NFIQ2Framework/NFIQ2Framework/include/stdint.h $
 * $Revision: #1 $
 * $Author: schwaiger $
 * 
 ******************************************************************************/

#ifndef STDINT_H
#define STDINT_H

#ifdef LINUX
#include "/usr/include/stdint.h"
#else

#ifndef __int8_t_defined
#define __int8_t_defined
typedef signed char int8_t;
typedef short int16_t;
typedef long int32_t;
#ifndef int64_t
typedef long long int64_t;
#endif
#endif

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
#ifndef __uint32_t_defined
#define __uint32_t_defined
typedef unsigned long uint32_t;
#endif
typedef unsigned long long uint64_t;

#endif /* LINUX */

#endif /* STDINT_H */
