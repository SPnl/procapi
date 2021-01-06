<?php

/**
 * Perform api request.
 */
function procapi_perform_request($path, $method, $accept_header, &$error, $headers = NULL, $data = NULL) {
  $error = FALSE;
  if (empty($headers)) {
    $token = procapi_get_oauth2_client_token(); 
    if (empty($token)) {
      procapi_error('Het was niet mogelijk een token op te halen bij Procurios.', NULL);
      $error = 'no token returned from api';
      return FALSE;
    }
    $headers = array(
      'Content-Type' => 'application/json',
      'Accept' => $accept_header,
      'Accept-Language' => 'en-US,en;q=0.5',
      'Authorization' => 'Bearer ' . $token,
    );
  }
  $options = array(
    'method' => $method,
    'headers' => $headers,
  );

  $server_domain = variable_get('procapi_procurios_server_domain');
  $url = 'https://' . $server_domain . $path;

  // Set parameters or data.
  if (!empty($data)) {
    switch($method) {
      case 'GET':
        $url = url($url, array('query' => $data));
        break;

      case 'POST':
        $options['data'] = json_encode($data);
        break;
    }
  }

  // Retry request three times when error.
  for ($i = 0; $i < 3; $i++) {
    $result = drupal_http_request($url, $options);
    if (in_array($result->code, array(200, 204))) {
      break;
    }
    sleep(2);
  }

  if (! in_array($result->code, array(200, 204))) {
    $error = $result;
    procapi_error('Er is een fout opgetreden bij een Procurios API request.', array('url' => $url, 'options' => $options, 'result' => $result));
    return FALSE;
  }
  
  if (procapi_is_json($result->data)) {
    $response = drupal_json_decode($result->data);
  }
  else {
    $response = $result->data;
  }
  return $response;
}

/**
 * Check if string is json.
 */
function procapi_is_json($string) {
  json_decode($string);
  return (json_last_error() == JSON_ERROR_NONE);
}

/**
 * Flatten properties.
 */
function procapi_flatten_properties($properties, &$fields, &$key, &$title) {
  if (!empty($properties)) {
    foreach ($properties as $prop_key => $property) {
      if (empty($property['type'])) {
        continue;
      }
      if (in_array($property['type'], array('string', 'integer', 'object'))) {
        if ($property['type'] == 'object' && !empty($property['properties'])) {
          $key = $key . $prop_key . '|';
          $title = $title . (empty($property['title']) ? $prop_key : $property['title']) . '|';
          procapi_flatten_properties($property['properties'], $fields, $key, $title);
        }
        else {
          $fields[$key . $prop_key] = $title . (empty($property['title']) ? $prop_key : $property['title']);
        }
      }
    }
  }
  $key = preg_replace('#[^|]*\|$#', '', $key);
  $title = preg_replace('#[^|]*\|$#', '', $title);
}


/*
 * Mail error to tdgraaff.
 */
function procapi_error($error_message, $debug_data) {
  global $base_url;
  $message = array(
    'content' => '<p>' . $error_message . '</p><p><pre>Debug data:<pre>@message_data</pre></p>',
    'data' => array(
      '@message_data' => print_r($debug_data, TRUE),
    ),
  );
  watchdog('procapi', $message['content'], $message['data'], WATCHDOG_ERROR);
  drupal_mail('procapi', 'error', 'tdgraaff@sp.nl', language_default(), $message, 'webmaster@sp.nl');
}
