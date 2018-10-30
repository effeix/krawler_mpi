#ifndef __CURLHANDLER_H__
#define __CURLHANDLER_H__

#include "curl/curl.h"
#include <string>

/*
 * By default, cURL writes responses to stdout. To prevent this and collect the data,
 * we need to setup a custom callback with the following prototype:
 *
 *     size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp);
 * 
 * Then we can use the callback to populate a custom buffer with the
 * contents of the response.
 * 
 * CURLOPT_WRITEFUNCTION is used to set the custom callback
 * CURLOPT_WRITEDATA is used to set the custom buffer we'll write to.
 * 
 * Sources:
 *     https://stackoverflow.com/a/9786295/4922572
 *     https://curl.haxx.se/libcurl/c/libcurl-tutorial.html
 */

// Custom callback for cURL
size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp);

std::string http_get(std::string url);

#endif/*__CURLHANDLER_H__*/
