#include <stdio.h>
#include "string.h"
#include "iot_os.h"
#include "iot_debug.h"
#include "iot_network.h"
#include "iot_socket.h"
#include "iot_fs.h"
#include "iot_flash.h"
#include "iot_pmd.h"
#include "net_sockets.h"
#include "entropy.h"
#include "ssl.h"
#include "ctr_drbg.h"
#include "entropy.h"
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include "debug.h"
#include "mbedtls/debug.h"
#include "iot_os.h"
#include "iot_socket.h"

#ifndef IPPROTO_TCP
#define IPPROTO_TCP     6
#endif

#define malloc(x) iot_os_malloc(x)
#define free(x)  iot_os_free(x)
#define read(fd,buf,len)        recv( fd, (char*)( buf ), (int)( len ), 0 )
#define write(fd,buf,len)       send( fd, (char*)( buf ), (int)( len ), 0 )
#define close(fd)               close(fd)
#ifdef errno
#undef errno
#define errno(x) socket_errno(x)
#endif // errno
// TLS operations
#define mbedtls_printf(fmt,args...) app_debug_print("[ssl]"fmt ,##args)


typedef enum{
	MBEDTLS_ERR_TLS_ERR_NO,
	MBEDTLS_ERR_TLS_ERR_MALLOC,
	MBEDTLS_ERR_TLS_ERR_CA_ERR,
	MBEDTLS_ERR_TLS_ERR_CONFIG_ERR,
	MBEDTLS_ERR_TLS_ERR_CONN_ERR,
	MBEDTLS_ERR_TLS_ERR_HANDSHAKE_ERR,
	MBEDTLS_ERR_TLS_ERR_SEND,
}E_MBEDTLS_ERR_TLS;


#define DEBUG_LEVEL 3

typedef struct{
	mbedtls_net_context client_fd;
	mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;
    mbedtls_x509_crt cacert;
}MBEDTLS_TLS_HANDLE;


static void tls_debug(void *tag, int ret, const char* file, int line, const char *str)
{
    //调试mbedtls栈时取消下列行的注释
	//app_debug_print("[ssl_debug]:ret=%d,func=%s,line=%d,str=%s",ret,file,line,str);
}

extern uint32_t ipaddr_addr(const char *);
 /*
 * Initiate a TCP connection with host:port and the given protocol
 */
static int mbedtls_net_connect_v2( mbedtls_net_context *ctx, const char *host,
                         unsigned short port, int proto )
{
    int ret = -1;
	struct sockaddr_in sa;
	memset(&sa, 0, sizeof(struct sockaddr_in));
  	sa.sin_family = AF_INET;

	sa.sin_addr.s_addr = ipaddr_addr(host);
  	if (sa.sin_addr.s_addr == OPENAT_INADDR_NONE)
    {
		struct hostent *hp = gethostbyname(host);
		if (!hp || !hp->h_addr)
			return MBEDTLS_ERR_NET_UNKNOWN_HOST;
      	sa.sin_addr = *(struct in_addr *)hp->h_addr;
    }
	sa.sin_port = ((port & 0xff) << 8) | ((port & 0xff00) >> 8);
	ctx->fd = socket(OPENAT_AF_INET, OPENAT_SOCK_STREAM, 0);
    if (ctx->fd < 0)
    {
    	ret = MBEDTLS_ERR_NET_SOCKET_FAILED;
        return ret;
    }
    if ((ret = connect(ctx->fd, (struct sockaddr *)&sa, sizeof(struct sockaddr))) < 0)
    {
        close(ctx->fd);
    }
	return ret;
}

 /*
 * Gracefully close the connection
 */
void mbedtls_net_free( mbedtls_net_context *ctx )
{
    if( ctx->fd == -1 )
        return;

    close( ctx->fd );

    ctx->fd = -1;
}

 /*
 * Initialize a context
 */
void mbedtls_net_init( mbedtls_net_context *ctx )
{
    ctx->fd = -1;
}
 /*
 * Check if the requested operation would be blocking on a non-blocking socket
 * and thus 'failed' with a negative return value.
 *
 * Note: on a blocking socket this function always returns 0!
 */
static int net_would_block( const mbedtls_net_context *ctx )
{
    int fd=ctx->fd;
    int err = socket_errno(fd);

    /*
     * Never return 'WOULD BLOCK' on a non-blocking socket
     */
    if( ( fcntl( ctx->fd, F_GETFL, 0 ) & O_NONBLOCK ) != O_NONBLOCK )
    {
        return( 0 );
    }

    switch( err )
    {
#if defined EAGAIN
        case EAGAIN:
#endif
#if defined EWOULDBLOCK && EWOULDBLOCK != EAGAIN
        case EWOULDBLOCK:
#endif
            return( 1 );
    }
    return( 0 );
}

 /*
 * Read at most 'len' characters
 */
int mbedtls_net_recv( void *ctx, unsigned char *buf, size_t len )
{
    int ret;
    int fd = ((mbedtls_net_context *) ctx)->fd;

    if( fd < 0 )
        return( MBEDTLS_ERR_NET_INVALID_CONTEXT );

    ret = (int) read( fd, buf, len );

    if( ret < 0 )
    {
        if( net_would_block( ctx ) != 0 )
            return( MBEDTLS_ERR_SSL_WANT_READ );

        if( socket_errno(fd) == EPIPE || socket_errno(fd) == ECONNRESET )
            return( MBEDTLS_ERR_NET_CONN_RESET );

        if( socket_errno(fd) == EINTR )
            return( MBEDTLS_ERR_SSL_WANT_READ );

        return( MBEDTLS_ERR_NET_RECV_FAILED );
    }

    return( ret );
}

 /*
 * Read at most 'len' characters, blocking for at most 'timeout' ms
 */
int mbedtls_net_recv_timeout( void *ctx, unsigned char *buf,
                              size_t len, uint32_t timeout )
{
    int ret;
    struct timeval tv;
    fd_set read_fds;
    int fd = ((mbedtls_net_context *) ctx)->fd;

    if( fd < 0 )
        return( MBEDTLS_ERR_NET_INVALID_CONTEXT );

    FD_ZERO( &read_fds );
    FD_SET( fd, &read_fds );

    tv.tv_sec  = timeout / 1000;
    tv.tv_usec = ( timeout % 1000 ) * 1000;

    ret = select( fd + 1, &read_fds, NULL, NULL, timeout == 0 ? NULL : &tv );

    /* Zero fds ready means we timed out */
    if( ret == 0 )
        return( MBEDTLS_ERR_SSL_TIMEOUT );

    if( ret < 0 )
    {

        if( socket_errno(fd) == EINTR )
            return( MBEDTLS_ERR_SSL_WANT_READ );

        return( MBEDTLS_ERR_NET_RECV_FAILED );
    }

    /* This call will not block */
    return( mbedtls_net_recv( ctx, buf, len ) );
}

 /*
 * Write at most 'len' characters
 */
int mbedtls_net_send( void *ctx, const unsigned char *buf, size_t len )
{
    int ret;
    int fd = ((mbedtls_net_context *) ctx)->fd;

    if( fd < 0 )
        return( MBEDTLS_ERR_NET_INVALID_CONTEXT );

    ret = (int) write( fd, buf, len );

    if( ret < 0 )
    {
        if( net_would_block( ctx ) != 0 )
            return( MBEDTLS_ERR_SSL_WANT_WRITE );


        if( socket_errno(fd) == EPIPE || socket_errno(fd) == ECONNRESET )
            return( MBEDTLS_ERR_NET_CONN_RESET );

        if( socket_errno(fd) == EINTR )
            return( MBEDTLS_ERR_SSL_WANT_WRITE );


        return( MBEDTLS_ERR_NET_SEND_FAILED );
    }

    return( ret );
}


static void app_mbedtls_free_context(MBEDTLS_TLS_HANDLE *handle)
{

	mbedtls_net_free( &handle->client_fd );

    mbedtls_x509_crt_free( &handle->cacert );
    mbedtls_ssl_free( &handle->ssl );
    mbedtls_ssl_config_free( &handle->conf );
    mbedtls_ctr_drbg_free( &handle->ctr_drbg );
    mbedtls_entropy_free( &handle->entropy );
	free(handle);

}


static int app_mbedtls_init(void **handleptr, const char *host, unsigned short port, const char *ca_crt,
                     unsigned int ca_crt_len)
{
	int ret = 1;
	E_MBEDTLS_ERR_TLS exit_code = MBEDTLS_ERR_TLS_ERR_NO;
	const char *pers = "ssl_client";
	*handleptr = (void *)malloc(sizeof(MBEDTLS_TLS_HANDLE));

	MBEDTLS_TLS_HANDLE *handle = (MBEDTLS_TLS_HANDLE *)*handleptr;
	memset(handle,0,sizeof(MBEDTLS_TLS_HANDLE));
	mbedtls_net_context *client_fd = &handle->client_fd;
	mbedtls_entropy_context *entropy = &handle->entropy;
	mbedtls_ctr_drbg_context *ctr_drbg = &handle->ctr_drbg;
	mbedtls_ssl_context *ssl = &handle->ssl;
	mbedtls_ssl_config *conf = &handle->conf;
	mbedtls_x509_crt *cacert = &handle->cacert;


	/*
	* 0. Initialize the RNG and the session data
	*/

	mbedtls_net_init( client_fd );
	mbedtls_ssl_init( ssl );
	mbedtls_ssl_config_init( conf );
	mbedtls_x509_crt_init( cacert );
	mbedtls_ctr_drbg_init( ctr_drbg );
	mbedtls_ssl_conf_dbg(conf,tls_debug,NULL);


#if defined(MBEDTLS_DEBUG_C)
	mbedtls_debug_set_threshold( DEBUG_LEVEL );
#endif
	mbedtls_printf( "  . Seeding the random number generator...\n" );



	mbedtls_entropy_init( entropy );
	if( ( ret = mbedtls_ctr_drbg_seed( ctr_drbg, mbedtls_entropy_func, entropy,
							   (const unsigned char *) pers,
							   strlen( pers ) ) ) != 0 )
	{
		mbedtls_printf( " failed\n	! mbedtls_ctr_drbg_seed returned %d\n", ret );
		exit_code = MBEDTLS_ERR_TLS_ERR_CONFIG_ERR;
		goto exit;
	}

	mbedtls_printf( " ok\n" );
	  /*
	 * 0. Initialize certificates
	 */
	mbedtls_printf( "  . Loading the CA root certificate ...\n" );

	if(ca_crt != NULL)
	{
		ret = mbedtls_x509_crt_parse( cacert, (const unsigned char *) ca_crt,
							  ca_crt_len+1);
		if( ret < 0 )
		{
			mbedtls_printf( " failed\n	!  mbedtls_x509_crt_parse returned -0x%x\n\n", -ret );
			exit_code = MBEDTLS_ERR_TLS_ERR_CA_ERR;
			goto exit;
		}
	}
	mbedtls_printf( " ok (%d skipped)\n", ret );

	/*
	 * 2. Setup stuff
	 */
	mbedtls_printf( "  . Setting up the SSL/TLS structure...\n" );

	if( ( ret = mbedtls_ssl_config_defaults( conf,
					MBEDTLS_SSL_IS_CLIENT,
					MBEDTLS_SSL_TRANSPORT_STREAM,
					MBEDTLS_SSL_PRESET_DEFAULT ) ) != 0 )
	{
		mbedtls_printf( " failed\n	! mbedtls_ssl_config_defaults returned %d\n\n", ret );
		exit_code = MBEDTLS_ERR_TLS_ERR_CONFIG_ERR;
		goto exit;
	}


	/* OPTIONAL is not optimal for security,
	 * but makes interop easier in this simplified example */
	mbedtls_ssl_conf_authmode( conf, MBEDTLS_SSL_VERIFY_NONE );
	mbedtls_ssl_conf_ca_chain( conf, cacert, NULL );
	mbedtls_ssl_conf_rng( conf, mbedtls_ctr_drbg_random, ctr_drbg );

	if( ( ret = mbedtls_ssl_setup( ssl, conf ) ) != 0 )
	{
		mbedtls_printf( " failed\n	! mbedtls_ssl_setup returned %d\n\n", ret );
		exit_code = MBEDTLS_ERR_TLS_ERR_CONFIG_ERR;
		goto exit;
	}

	if( ( ret = mbedtls_ssl_set_hostname( ssl, host ) ) != 0 )
	{
		mbedtls_printf( " failed\n	! mbedtls_ssl_set_hostname returned %d\n\n", ret );
		exit_code = MBEDTLS_ERR_TLS_ERR_CONFIG_ERR;
		goto exit;
	}
	mbedtls_printf( " ok\n" );

	mbedtls_ssl_set_bio( ssl, client_fd, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout );

	return (int)exit_code;
	exit:

	app_mbedtls_free_context(handle);
	return (int)exit_code;
}

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
int app_mbedtls_connect(void **handle, const char *host, unsigned short port, const char *ca_crt,
                     unsigned int ca_crt_len, unsigned int timeout_ms)
{
    int ret = 1;
    E_MBEDTLS_ERR_TLS exit_code = MBEDTLS_ERR_TLS_ERR_NO;


    uint32_t flags;
	mbedtls_printf("  . tls init ....\n");
	if((exit_code = app_mbedtls_init(handle,host,port,ca_crt,ca_crt_len))  != MBEDTLS_ERR_TLS_ERR_NO)
		return exit_code;

	MBEDTLS_TLS_HANDLE *wxpp_handle = (MBEDTLS_TLS_HANDLE*)*handle;
	/*
     * 1. Start the connection
     */
    mbedtls_printf( "  . Connecting to tcp/%s/%d...\n", host, port );

    if( ( ret = mbedtls_net_connect_v2( &wxpp_handle->client_fd, host,
                                         port, MBEDTLS_NET_PROTO_TCP ) ) != 0 )
    {
        mbedtls_printf( " failed\n  ! mbedtls_net_connect returned %d\n\n", ret );
		exit_code = MBEDTLS_ERR_TLS_ERR_CONN_ERR;
        goto exit;
    }
	mbedtls_printf( " ok\n" );
    /*
     * 4. Handshake
     */
    mbedtls_printf( "  . Performing the SSL/TLS handshake...\n" );

    while( ( ret = mbedtls_ssl_handshake( &wxpp_handle->ssl ) ) != 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            mbedtls_printf( " failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", -ret );
			exit_code = MBEDTLS_ERR_TLS_ERR_HANDSHAKE_ERR;
            goto exit;
        }
    }

    mbedtls_printf( " ok\n" );

    /*
     * 5. Verify the server certificate
     */
    mbedtls_printf( "  . Verifying peer X.509 certificate...\n" );

    /* In real life, we probably want to bail out when ret != 0 */
    if( ( flags = mbedtls_ssl_get_verify_result( &wxpp_handle->ssl ) ) != 0 )
    {
        char vrfy_buf[512];

        mbedtls_printf( " failed\n" );

        mbedtls_x509_crt_verify_info( vrfy_buf, sizeof( vrfy_buf ), "  ! ", flags );

        mbedtls_printf( "%s\n", vrfy_buf );
    }
    else
        mbedtls_printf( " ok\n" );


    return (int)exit_code;

exit:

    app_mbedtls_free_context(wxpp_handle);

    return (int)exit_code;

}

/**
 * @brief disconnect a TLS connection
 * @param handle TLS connection handle
 */
void app_mbedtls_disconnect(void *handle)
{
	MBEDTLS_TLS_HANDLE *hand = (MBEDTLS_TLS_HANDLE *)handle;
	mbedtls_net_free( &hand->client_fd );
    mbedtls_x509_crt_free( &hand->cacert );
    mbedtls_ssl_free( &hand->ssl );
    mbedtls_ssl_config_free( &hand->conf );
    mbedtls_ctr_drbg_free( &hand->ctr_drbg );
    mbedtls_entropy_free( &hand->entropy );
	free(hand);
}
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
int app_mbedtls_write(void *handle, const void *data, size_t data_len, unsigned int timeout_ms,
                   int *written_len)
{
	MBEDTLS_TLS_HANDLE *hand = (MBEDTLS_TLS_HANDLE *)handle;
	*written_len = mbedtls_ssl_write( (void*)&hand->ssl, data, data_len );
	if(*written_len >= 0)
	{
		return MBEDTLS_ERR_TLS_ERR_NO;
	}
	else
	{
		return MBEDTLS_ERR_TLS_ERR_SEND;
	}
}
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
int app_mbedtls_read(void *handle, void *buffer, size_t buffer_len, unsigned int timeout_ms,
                  int *read_len)
{
 	MBEDTLS_TLS_HANDLE *hand = (MBEDTLS_TLS_HANDLE *)handle;
	*read_len = mbedtls_ssl_read((void*)&hand->ssl, buffer,buffer_len);
	if(*read_len >=0)
	{
		return MBEDTLS_ERR_TLS_ERR_NO;
	}
	else
	{
		return MBEDTLS_ERR_TLS_ERR_SEND;
	}
}

