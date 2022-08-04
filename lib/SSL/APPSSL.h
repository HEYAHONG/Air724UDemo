#ifndef APPSSL_H_INCLUDED
#define APPSSL_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus
#include "stdlib.h"
#include "mbedtls/ssl.h"
/**
 * @brief make a TLS connection
 *
 * @param handle        TLS connection handle
 * @param host          server host
 * @param port          server port
 * @param ca_crt        CA certificate in PEM format
 * @param ca_crt_len    CA certificate length
 * @param timeout_ms    timeout in millisecond
 *
 * @return 0 if success, or MBEDTLS_ERR_TLS_xxx if error
 */
int app_mbedtls_connect(void **handle, const char *host, unsigned short port, const char *ca_crt,unsigned int ca_crt_len, unsigned int timeout_ms);

/**
 * @brief disconnect a TLS connection
 * @param handle TLS connection handle
 */
void app_mbedtls_disconnect(void *handle);

/**
 * @brief write to a TLS connection
 *
 * @param handle        TLS connection handle
 * @param data          data to write
 * @param data_len      data size to write
 * @param timeout_ms    write timeout in millisecond
 * @param written_len   bytes written
 *
 * @return 0 if success, or MBEDTLS_ERR_TLS_xxx if error

 */
int app_mbedtls_write(void *handle, const void *data, size_t data_len, unsigned int timeout_ms,int *written_len);

/**
 * @brief block read from TLS connection
 *
 * @param handle        TLS connection handle
 * @param buffer        output buffer
 * @param buffer_len    output buffer size
 * @param timeout_ms    read timeout in millisecond
 * @param read_len      bytes read
 *
 * @return 0 if success, or MBEDTLS_ERR_TLS_xxx if error
 */
int app_mbedtls_read(void *handle, void *buffer, size_t buffer_len, unsigned int timeout_ms,int *read_len);

#ifdef __cplusplus
};
#endif // __cplusplus

#endif // APPSSL_H_INCLUDED
