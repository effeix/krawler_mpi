#include "curlhandler.h"
#include <iostream>

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
size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp) {
    // buffer is a pointer to the response contents
    ((std::string*)userp)->append((char*)buffer, size * nmemb);
    return size * nmemb;
}

std::string http_get(std::string url) {
    CURL *curl;
    std::string buffer;

    curl = curl_easy_init();

    if(curl) {

        // cURL uses C strings instead of the string object
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // If the url suffers a redirection, we tell cURL to follow it
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // Set custom callback to be used
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);

        // Set which variable will be used by the custom callback
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

        // Perform the request
        CURLcode code = curl_easy_perform(curl);

        // Check for failure
        if(code != CURLE_OK) {
            fprintf(stderr, "curl failed: %s\n", curl_easy_strerror(code));
        }

        // Clean all options
        curl_easy_cleanup(curl);
    }
    
    return buffer;
}
