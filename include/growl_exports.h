#ifndef GROWL_EXPORTS_H
#define GROWL_EXPORTS_H

#ifdef _WIN32
  #ifndef GROWL_STATIC
    #ifdef GROWL_DLL
      #define GROWL_EXPORT __declspec(dllexport)
    #else
      #define GROWL_EXPORT __declspec(dllimport)
    #endif
  #else
    #define GROWL_EXPORT
  #endif
#else
  #define GROWL_EXPORT
#endif /*_WIN32 */


#ifdef _WIN32
#ifndef GROWL_CPP_STATIC
#ifdef GROWL_CPP_DLL
#define GROWL_CPP_EXPORT __declspec(dllexport)
#else
#define GROWL_CPP_EXPORT __declspec(dllimport)
#endif
#else
#define GROWL_CPP_EXPORT
#endif
#else
#define GROWL_CPP_EXPORT
#endif /*_WIN32*/


#endif /*GROWL_EXPORTS_H*/
