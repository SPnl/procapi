<?php

// Oauth.
define("PROCAPI_OAUTH_TOKEN_PATH", '/l/oauth2/token');

/**
 * Get oauth2 client token.
 */
function procapi_get_oauth2_client_token() {
  $server_domain = variable_get('procapi_procurios_server_domain');
  $client_id = variable_get('procapi_oauth_client_id');
  $client_secret = variable_get('procapi_oauth_client_secret');
  $scope = variable_get('procapi_oauth_scope', '');

  if (
    !empty($server_domain) &&
    !empty($client_id) &&
    !empty($client_secret) &&
    !empty($scope)
  ) {
    $oauth2_config = array(
      'token_endpoint' => 'https://' . $server_domain . PROCAPI_OAUTH_TOKEN_PATH,
      'auth_flow' => 'client-credentials',
      'client_id' => $client_id,
      'client_secret' => $client_secret,
      'scope' => $scope,
    );

    try {
      $oauth2_client = new OAuth2\ProcClient($oauth2_config, $client_id);
      $access_token = $oauth2_client->getAccessToken();
    }
    catch (Exception $e) {
      drupal_set_message($e->getMessage(), 'error');
    } 
    if (!empty($access_token)) {
      return $access_token;
    }
  }
  drupal_set_message('Failed to get token from Procurios');
  return FALSE;
}


