
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>


#define LIBSSH_STATIC 1
#include <libssh/libssh.h>
#include <libssh/callbacks.h>

#include "handlers.h"
#include "fluxui.h"

ssh_session flux_ssh_session = NULL;


/**
 * Handle a call to runtest() made by JavaScript.
 *
 * takes 1 parameters:
 *   0: callback id
 * on success, returns a result in |output| separated by \1:
 *   0: callback id
 *   1: libssh version string
 * on internal error, returns an error string in |output|.
 *
 * @param[in] num_params The number of params in |params|.
 * @param[in] params An array of strings, parameters to this function.
 * @param[out] output A string to write informational function output to.
 * @return An errorcode; 0 means success, anything else is a failure.
 */
int HandleRunTest( int num_params, char **params, char **output ) {
  const char *version;
  const char *callback_id;
  int output_len;

  fprintf( stderr, "HandleRunTest() called\n" );

  if ( num_params != 1 ) {
    *output = PrintfToNewString( "runtest takes 1 parameters." );
    return 1;
  }

  callback_id = params[0];

  version = ssh_version(0);
  if ( version == NULL ) {
    version = "unknown";
  }

  output_len = strlen( callback_id ) + 1
    + strlen( version ) + 1;

  *output = (char *) calloc( output_len, 1 );
  if (!*output) {
    *output = PrintfToNewString( "out of memory." );
    return 3;
  }
  snprintf( *output, output_len, "%s\1%s", callback_id, version );

  return 0;

}


/**
 * Handle a call to connectToFlux() made by JavaScript.
 *
 * takes 1 parameters:
 *   0: callback id
 * on success, returns a result in |output| separated by \1:
 *   0: callback id
 *   1: status string: "OK" or error message
 * on internal error, returns an error string in |output|.
 *
 * @param[in] num_params The number of params in |params|.
 * @param[in] params An array of strings, parameters to this function.
 * @param[out] output A string to write informational function output to.
 * @return An errorcode; 0 means success, anything else is a failure.
 */
int HandleConnectToFlux( int num_params, char **params, char **output ) {
  const char *version;
  const char *callback_id;
  const char *error_msg;
  int output_len;
  int verbosity;
  int rc;
  unsigned int port;
  ssh_key key;
  unsigned char *hash = NULL;
  size_t hlen;
  const char *banner;

  fprintf( stderr, "HandleConnectToFlux() called\n" );

  if ( num_params != 1 ) {
    fprintf( stderr, "HandleConnectToFlux() takes 1 parameters\n" );
    *output = PrintfToNewString( "connectToFlux takes 1 parameters" );
    return -1;
  }

  if ( flux_ssh_session != NULL ) {
    fprintf( stderr, "HandleConnectToFlux(): already connected\n" );
    *output = PrintfToNewString( "connectToFlux: already connected" );
    return -1;
  }

  fprintf( stderr, "HandleConnectToFlux(): creating session\n" );
  flux_ssh_session = ssh_new();
  if ( flux_ssh_session == NULL ) {
    fprintf( stderr, "HandleConnectToFlux(): session creation failed\n" );
    *output = PrintfToNewString( "connectToFlux: session creation failed" );
    return -1;
  }

  fprintf( stderr, "HandleConnectToFlux(): setting options\n" );
  rc = ssh_options_set( flux_ssh_session, SSH_OPTIONS_HOST, "127.0.0.1" );
  if ( rc != 0 ) {
    perror( "HandleConnectToFlux: perror: " );
    error_msg = ssh_get_error( flux_ssh_session );
    fprintf( stderr, "HandleConnectToFlux(): setting host failed: %s\n", error_msg );
    *output = PrintfToNewString( "connectToFlux: setting host failed: %s", error_msg );
    goto error_cleanup1;
  }

  port = 22;
  rc = ssh_options_set( flux_ssh_session, SSH_OPTIONS_PORT, &port );
  if ( rc != 0 ) {
    error_msg = ssh_get_error( flux_ssh_session );
    fprintf( stderr, "HandleConnectToFlux(): setting port failed: %s\n", error_msg );
    *output = PrintfToNewString( "connectToFlux: setting port failed: %s", error_msg );
    goto error_cleanup1;
  }

  // TODO: get username from JavaScript
  rc = ssh_options_set( flux_ssh_session, SSH_OPTIONS_USER, "markmont" );
  if ( rc != 0 ) {
    error_msg = ssh_get_error( flux_ssh_session );
    fprintf( stderr, "HandleConnectToFlux(): setting user failed: %s\n", error_msg );
    *output = PrintfToNewString( "connectToFlux: setting user failed: %s", error_msg );
    goto error_cleanup1;
  }

  // The directory has to be set even if we don't use it, or ssh_connect()
  // will fail.
  rc = ssh_options_set( flux_ssh_session, SSH_OPTIONS_SSH_DIR, "/" );
  if ( rc != 0 ) {
    error_msg = ssh_get_error( flux_ssh_session );
    fprintf( stderr, "HandleConnectToFlux(): setting ssh directory failed: %s\n", error_msg );
    *output = PrintfToNewString( "connectToFlux: setting ssh directory failed: %s", error_msg );
    goto error_cleanup1;
  }

  verbosity = SSH_LOG_PROTOCOL;
  verbosity = SSH_LOG_FUNCTIONS;
  rc = ssh_options_set( flux_ssh_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity );
  if ( rc != 0 ) {
    error_msg = ssh_get_error( flux_ssh_session );
    fprintf( stderr, "HandleConnectToFlux(): setting verbosity failed: %s\n", error_msg );
    *output = PrintfToNewString( "connectToFlux: setting verbosity failed: %s", error_msg );
    goto error_cleanup1;
  }

  // TODO (?) set log callback function
  //ssh_set_log_callback( ... );

  // TODO: SSH_OPTIONS_STATUS_CALLBACK
  //int port = 22;
  //ssh_options_set(flux_ssh_session, SSH_OPTIONS_PORT, &port);

  fprintf( stderr, "HandleConnectToFlux(): connecting\n" );
  rc = ssh_connect( flux_ssh_session );
  if ( rc != SSH_OK ) {
    error_msg = ssh_get_error( flux_ssh_session );
    fprintf( stderr, "HandleConnectToFlux(): connect failed: %s\n", error_msg );
    *output = PrintfToNewString( "connectToFlux: connect failed: %s", error_msg );
    ssh_free( flux_ssh_session );
    flux_ssh_session = NULL;
    return -1;
  }

  fprintf( stderr, "HandleConnectToFlux(): getting public key\n" );
  rc = ssh_get_publickey( flux_ssh_session, &key );
  if ( rc != SSH_OK ) {
    error_msg = ssh_get_error( flux_ssh_session );
    fprintf( stderr, "HandleConnectToFlux(): getting public key failed: %s\n", error_msg );
    *output = PrintfToNewString( "connectToFlux: getting public key failed: %s", error_msg );
    goto error_cleanup;
  }

  fprintf( stderr, "HandleConnectToFlux(): getting public key hash\n" );
  rc = ssh_get_publickey_hash( key, SSH_PUBLICKEY_HASH_SHA1, &hash, &hlen );
  ssh_key_free( key );
  if ( rc != SSH_OK ) {
    error_msg = ssh_get_error( flux_ssh_session );
    fprintf( stderr, "HandleConnectToFlux(): getting public key failed: %s\n", error_msg );
    *output = PrintfToNewString( "connectToFlux: getting public key failed: %s", error_msg );
    goto error_cleanup;
  }

  ssh_print_hexa( "Public key hash", hash, hlen );
  ssh_clean_pubkey_hash( &hash );


  fprintf( stderr, "HandleConnectToFlux(): userauth_none\n" );
  rc = ssh_userauth_none( flux_ssh_session, NULL );
  if ( rc != SSH_OK ) {
    error_msg = ssh_get_error( flux_ssh_session );
    fprintf( stderr, "HandleConnectToFlux(): userauth_none failed: %s\n", error_msg );
    *output = PrintfToNewString( "connectToFlux: userauth_none failed: %s", error_msg );
    goto error_cleanup;
  }

  // TODO: ssh_get_issue_banner() ?
  fprintf( stderr, "HandleConnectToFlux(): getting server banner\n" );
  banner = ssh_get_serverbanner( flux_ssh_session );
  if( banner ) {
      fprintf( stderr, "HandleConnectToFlux: banner: %s\n", banner );
      free( (void *) banner );
  }

// HERE

  callback_id = params[0];

  version = "yatta ne!";

  output_len = strlen( callback_id ) + 1
    + strlen( version ) + 1;

  *output = (char *) calloc( output_len, 1 );
  if (!*output) {
    *output = PrintfToNewString( "out of memory" );
    return -1;
  }
  snprintf( *output, output_len, "%s\1%s", callback_id, version );

  fprintf( stderr, "HandleConnectToFlux(): done, returning\n" );
  return 0;

  error_cleanup:
  ssh_disconnect( flux_ssh_session );
  error_cleanup1:
  ssh_free( flux_ssh_session );
  flux_ssh_session = NULL;
  return -1;

}


/**
 * Handle a call to disconnectFromFlux() made by JavaScript.
 *
 * takes 1 parameters:
 *   0: callback id
 * on success, returns a result in |output| separated by \1:
 *   0: callback id
 *   1: status string: "OK" or error message
 * on internal error, returns an error string in |output|.
 *
 * @param[in] num_params The number of params in |params|.
 * @param[in] params An array of strings, parameters to this function.
 * @param[out] output A string to write informational function output to.
 * @return An errorcode; 0 means success, anything else is a failure.
 */
int HandleDisconnectFromFlux( int num_params, char **params, char **output ) {
  const char *version;
  const char *callback_id;
  int output_len;

  fprintf( stderr, "HandleDisconnectFromFlux() called\n" );

  if ( num_params != 1 ) {
    fprintf( stderr, "HandleDisconnectFromFlux() takes 1 parameters\n" );
    *output = PrintfToNewString( "disconnectFromFlux takes 1 parameters" );
    return -1;
  }

  if ( flux_ssh_session == NULL ) {
    fprintf( stderr, "HandleDisconnectFromFlux(): not connected\n" );
    *output = PrintfToNewString( "disconnectFromFlux: not connected" );
    return -1;
  }

  ssh_disconnect( flux_ssh_session );
  ssh_free( flux_ssh_session );
  flux_ssh_session = NULL;

  callback_id = params[0];

  version = "owari desu";

  output_len = strlen( callback_id ) + 1
    + strlen( version ) + 1;

  *output = (char *) calloc( output_len, 1 );
  if (!*output) {
    *output = PrintfToNewString( "out of memory" );
    return -1;
  }
  snprintf( *output, output_len, "%s\1%s", callback_id, version );

  fprintf( stderr, "HandleDisconnectFromFlux(): done, returning\n" );
  return 0;

}

