#include "module_rest.h"
#include "../base_module.h"


int cb_index (const struct _u_request *request, struct _u_response *response, void *user_data)
{
    ulfius_set_string_body_response(response, 200, "Index");
    return U_CALLBACK_CONTINUE;
}

int cb_test (const struct _u_request *request, struct _u_response *response, void *user_data)
{
    ulfius_set_string_body_response(response, 200, "Test");
    return U_CALLBACK_CONTINUE;
}

int main(void)
{
    struct _u_instance instance;

    if (ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK) {
        fprintf(stderr, "Error ulfius_init_instance, abort\n");
        return(1);
    }

    ulfius_add_endpoint_by_val(&instance, "GET", "/", NULL, 0, &cb_index, NULL);
    ulfius_add_endpoint_by_val(&instance, "GET", "/test", NULL, 0, &cb_test, NULL);

    if (ulfius_start_framework(&instance) == U_OK) {
        printf("Start framework on port %d\n", instance.port);

        getchar();
    } else {
        fprintf(stderr, "Error starting framework\n");
    }

    printf("End framework\n");

    ulfius_stop_framework(&instance);
    ulfius_clean_instance(&instance);

    return 0;
}