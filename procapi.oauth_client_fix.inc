<?php

namespace OAuth2;

class ProcClient extends Client {
  protected function getToken($data) {
    // Implement the custom logic that is needed by the oauth2 server.
    if (array_key_exists('scope', $data) && is_null($data['scope'])) {
      unset($data['scope']);
    }

    $client_id = $this->params['client_id'];
    $client_secret = $this->params['client_secret'];
    $token_endpoint = $this->params['token_endpoint'];

    $data['client_id'] = $client_id;
    $data['client_secret'] = $client_secret;

    $options = array(
      'method' => 'POST',
      'data' => drupal_http_build_query($data),
      'headers' => array(
        'Content-Type' => 'application/x-www-form-urlencoded',
      ),
    );
    if ($this->params['skip-ssl-verification']) {
      $options['context'] = stream_context_create(array(
        'ssl' => array(
          'verify_peer' => FALSE,
          'verify_peer_name' => FALSE,
        ),
      ));
    }
    $result = drupal_http_request($token_endpoint, $options);

    if ($result->code != 200) {
      throw new \Exception(
        t("Failed to get an access token of grant_type @grant_type.\nError: @result_error",
        array(
          '@grant_type' => $data['grant_type'],
          '@result_error' => $result->error,
        ))
      );
    }

    $token = drupal_json_decode($result->data);

    if (!isset($token['expires_in'])) {
      $token['expires_in'] = 3600;
    }

    return $token;
  }
}

