#ifndef __MODULE_REST_H__
#define __MODULE_REST_H__

#include <stdio.h>
#include <ulfius.h>

// https://github.com/babelouest/ulfius

#define PORT 8080

/**
 * @brief API endpoint "/"
 */
int cb_index (const struct _u_request *request, struct _u_response *response, void *user_data);
/**
 * @brief API endpoint "/test"
 */
int cb_test (const struct _u_request *request, struct _u_response *response, void *user_data);


#endif // __MODULE_REST_H__