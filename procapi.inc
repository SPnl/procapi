<?php

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
  return FALSE;
}

/**
 * Flatten properties.
 */
function procapi_flatten_properties($properties, &$fields, &$key, &$title) {
  foreach ($properties as $prop_key => $property) {
    if (empty($property['type'])) {
      continue;
    }
    if (in_array($property['type'], array('string', 'integer', 'object'))) {
      if ($property['type'] == 'object' && isset($property['properties'])) {
        $key = $key . $prop_key . '|';
        $title = $title . (empty($property['title']) ? $prop_key : $property['title']) . '|';
        procapi_flatten_properties($property['properties'], $fields, $key, $title);
      }
      else {
        $fields[$key . $prop_key] = $title . (empty($property['title']) ? $prop_key : $property['title']);
      }
    }
  }
  $key = preg_replace('#[^|]*\|$#', '', $key);
  $title = preg_replace('#[^|]*\|$#', '', $title);
}

/**
 * Get form schema fields
 */
function procapi_get_proc_form_schema_contact_fields($form_id) {
  $schema = procapi_get_proc_form_schema($form_id);
  $properties = $schema[$form_id]['properties'];
  $raw_fields = array();
  $key = '';
  $title = '';
  procapi_flatten_properties($properties, $raw_fields, $key, $title);
  foreach($raw_fields as $key => $title) {
    $fields[$key] = str_replace('|', ' - ', $title);
  }
  return $fields;
}

/**
 * Get api form schema.
 */
function procapi_get_proc_form_schema($form_id) {
  $schema = &drupal_static(__FUNCTION__);
  if (!isset($schema[$form_id])) {
    $schema[$form_id] = NULL;
    if ($cache = cache_get('procapi_schema_' . $form_id)) {
      $schema[$form_id] = $cache->data;
    }
    else {
      // Get registration set.
      $registration_set = procapi_get_proc_registration_set();
      $path = $registration_set[$form_id]['_links']['schema']['href'];
      $method = $registration_set[$form_id]['_links']['schema']['type'];
      $result = procapi_perform_request($path, $method, $error);
      if (!empty($result)) {
        if (is_array($result) && !empty($result['title'])) {
          $schema[$form_id] = $result;
          cache_set('procapi_schema_' . $form_id, $schema[$form_id], 'cache', time() + 360);
        }
      }
    }
  }
  return $schema;
}

/**
 * Validate contact api object.
 */
function procapi_proc_validate_object($object, $form_id, &$error) {
  // Get registration set.
  $registration_set = procapi_get_proc_registration_set();
  $path = $registration_set[$form_id]['_links']['validate']['href'];
  $method = $registration_set[$form_id]['_links']['validate']['type'];
  $result = procapi_perform_request($path, $method, $error, NULL, json_encode($object));
  return $result;
}

/**
 * Push contact api object.
 */
function procapi_proc_push_object($object, $form_id, &$error) {
  // Get registration set.
  $registration_set = procapi_get_proc_registration_set();
  $path = $registration_set[$form_id]['_links']['registration']['href'];
  $method = $registration_set[$form_id]['_links']['registration']['type'];
  $result = procapi_perform_request($path, $method, $error, NULL, json_encode($object));
  return $result;
}

/**
 * Get registration set list.
 */
function procapi_get_proc_forms_list() {
  $list = &drupal_static(__FUNCTION__);
  if (!isset($list)) {
    $list = array();
    $registration_set = procapi_get_proc_registration_set();
    foreach ($registration_set as $registration_form) {
      $list[$registration_form['id']] = $registration_form['title'];
    }
  }
  return $list;
}

/**
 * Get registration set.
 */
function procapi_get_proc_registration_set() {
  $registration_set = &drupal_static(__FUNCTION__);
  if (!isset($registration_set)) {
    $registration_set = array();
    if ($cache = cache_get('procapi_registration_set')) {
      $registration_set = $cache->data;
    }
    else {
      $token = procapi_get_oauth2_client_token();
      if (!empty($token)) {
        // Get registration set.
        $server_domain = variable_get('procapi_procurios_server_domain');
        $path = PROCAPI_API_REGISTRATION_SET_PATH;
        $result = procapi_perform_request($path, 'GET', $error);
        if (!empty($result)) {
          if (!empty($result['registrationSet'])) {
            foreach($result['registrationSet'] as $registration_form) {
              $registration_set[$registration_form['id']] = $registration_form;
            }
            cache_set('procapi_registration_set', $registration_set, 'cache', time() + 360);
          }
        }
      }
    }
  }
  return $registration_set;
}

/**
 * Perform api request.
 */
function procapi_perform_request($path, $method, &$error, $headers = NULL, $data = NULL) {
  if (empty($headers)) {
    $token = procapi_get_oauth2_client_token(); 
    $headers = array(
      'Content-Type' => 'application/json',
      'Accept' => 'application/vnd.procurios.profile+json; version=1',
      'Accept-Language' => 'en-US,en;q=0.5',
      'Authorization' => 'Bearer ' . $token,
    );
  }
  $options = array(
    'method' => $method,
    'headers' => $headers,
  );
  if (!empty($data)) {
    $options['data'] = $data;
  }
  $server_domain = variable_get('procapi_procurios_server_domain');
  $url = 'https://' . $server_domain . $path;

  $result = drupal_http_request($url, $options);

  if ($result->code != 200) {
    $error = $result;
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
  drupal_mail('spwebformsync', 'error', 'tdgraaff@sp.nl', language_default(), $message, 'webmaster@sp.nl');
}

/**
 * Split name.
 */
function procapi_split_name($name) {
  $matches = array();
  $name = trim($name);
  $name = trim($name, '.,');

  // Remove comma from name.
  $comma_pos = strpos($name, ',');
  if ($comma_pos !== FALSE) {
    $first_half = trim(substr($name, 0, $comma_pos));
    $last_half = trim(substr($name, $comma_pos + 1));
    $name = $last_half . ' ' . $first_half;
  }

  // Replace terms.
  $replace = array(
    'v' => 'van',
    'v.' => 'van',
    'vd' => 'van de',
    'vd.' => 'van de',
    'v.d.' => 'van de',
    'v/d' => 'van de',
  );
  foreach ($replace as $orig => $new) {
    if (strpos($name, ' ' . $orig . ' ') !== FALSE) {
      $name = str_replace($orig, $new, $name);
    }
  }

  // Regexes for title and middle names.
  $titles_regex = 'Mr|mr|Mevr|mevr|Dhr|dhr|Ir|ir|Ing|ing|Bc|bc|Ms|ms|Drs|drs|Dr|dr';

  $middle_names_array = procapi_middle_names();
  $middle_names_regex = implode('|', $middle_names_array);

  // Split name on middle_name.
  $regex = "@^(?'title'(\b(" . $titles_regex . ")\b\.?\s)?)(?'first_name'(([A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'\-\.]+)\s*)+?)(?'middle_name'(\b(" . $middle_names_regex . ")\b\s*))(?'last_name'(\b[A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'][a-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'\-]+\s?)+)$@";
  $result = preg_match($regex, $name, $matches);
  if (!preg_match($regex, $name, $matches)) {
    // Split name on middle name, last name first.
    $regex = "@^(?'title'(\b(" . $titles_regex . ")\b\.?\s)?)(?'last_name'(\b[A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'][A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'\-]+\s?)+),?\s(?'first_name'(([A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'\-\.]+)\s*)+?)(?'middle_name'(\b(" . $middle_names_regex . ")\b\s*))$@";
    $result = preg_match($regex, $name, $matches);
    if (!preg_match($regex, $name, $matches)) {
      // Split name without middle_name in first name and last name(s).
      $regex = "@^(?'title'(\b(" . $titles_regex . ")\b\.?\s)?)(?'first_name'(([A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'\-\.]+)\s*)+?)(?'last_name'(\b[A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'][A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'\-]+\s?)+)$@";
      $result = preg_match($regex, $name, $matches);
      if (!preg_match($regex, $name, $matches)) {
        // Split name in first name and last name, last name first.
        $regex = "@^(?'title'(\b(" . $titles_regex . ")\b\.?\s)?)(?'last_name'(\b[A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'][A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'\-]+\s?)+),?\s+(?'first_name'((([A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ']\.)+)\s*)+?)$@";
        $result = preg_match($regex, $name, $matches);
        if (!preg_match($regex, $name, $matches)) {
          // Check if only first name.
          $regex = "@^(?'first_name'([A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'][a-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'\-]+))$@";
          $result = preg_match($regex, $name, $matches);
          if (!preg_match($regex, $name, $matches)) {
            // No match found.
            return FALSE;
          }
        }
      }
    }
  }
  // Set first character uppercase.
  $parts = array(
    'first_name',
    'last_name',
  );
  foreach ($parts as $part) {
    if (isset($matches[$part])) {
      $matches[$part] = trim($matches[$part]);
      if (!empty($matches[$part])) {
        $matches[$part] = ucfirst($matches[$part]);
      }
    }
  }
  if (empty($matches['last_name']) && !empty($matches['first_name'])) {
    $matches['last_name'] = $matches['first_name'];
    $matches['first_name'] = '';
   }
  return $matches;
}

/**
 * Longer middle names first.
 */
function procapi_middle_names() {
  return array("Uijt te de","van van de","uijt te de","Voor in ‘t","De van der","de van der","Van van de","voor in ‘t","Uit te de","onder den","Voor in t","onder het","van de l’","De die le","de die le","Van de l’","voor in t","Onder het","Onder den","uit te de","over het","uijt den","Onder de","boven d’","uijt ten","Uijt ten","over den","Over den","onder de","onder ‘t","Onder ‘t","Van de l","Boven d’","van de l","voor den","Over het","Uijt den","Voor den","Over ‘t","Boven d","Von dem","boven d","Von der","von den","Von den","von der","von dem","Onder t","van der","Van ter","Van gen","van het","van gen","Van het","van ter","over ‘t","van den","Van den","Van Der","Van der","onder t","Van Den","bij den","auf dem","Auf den","Auf dem","voor de","uit het","Uit het","auf den","Auf der","Voor de","voor ‘t","auf ter","Auf ter","auf der","aan het","Aan het","uijt de","vor der","Uijt de","uijt ‘t","Uijt ‘t","Vor der","Uit ten","Aan der","aan der","aan den","Aan den","uit ten","uit den","Uit den","aus der","Bij het","Bij den","Voor ‘t","over de","Over de","bij het","Aus dem","aus dem","Aus den","Aus der","aus den","in het","die le","In der","in der","In het","Uit de","van ‘t","Van de","van De","van de","Van ‘t","Uit ‘t","In den","uit de","uit ‘t","in den","Op ten","op het","Over t","Op het","Die le","op gen","over t","op ten","Von ‘t","Bij ‘t","aus ‘m","bij ‘t","Bij de","von ‘t","bij de","Op gen","op der","Aan ‘t","aan ‘t","De las","de las","Van la","van la","Aan de","aan de","op den","Op der","De die","de die","Op den","Aus ‘m","Onder","op ‘t","Op ‘t","Op de","op de","onder","Uit t","Van t","De l’","uit t","Dalla","de l’","De la","de le","De le","de la","Von t","bij t","Am de","aan t","Aan t","am de","Aus m","Bij t","von t","aus m","Degli","dalla","van t","In de","in ‘t","In ‘t","della","in de","unter","Unter","Della","degli","deca","uijt","Over","over","Voor","in t","In t","Uijt","voor","op t","De l","thoe","de l","thor","Deca","Thor","Dal’","Thoe","Op t","dal’","zum","Zur","toe","zur","tot","Toe","Tot","uit","vom","Von","Vom","ver","Ver","van","Van","vor","Vor","Uit","von","Zum","Los","Den","den","del","Del","Dei","dei","Der","der","dos","Het","Dos","don","Des","des","das","Das","Aus","aus","auf","Auf","Aan","aan","Ben","ben","Dal","dal","bin","Bin","Bij","bij","het","Don","Ter","las","ter","ten","Ten","los","les","Les","Tho","Las","tho","d’","‘T","‘t","D’","Da","da","l’","op","S’","‘s","s’","te","af","am","Am","al","Af","de","Al","zu","Zu","to","Te","De","du","El","Du","Le","do","Op","el","la","in","L’","In","im","La","Im","Do","To","lo","di","‘S","Of","Lo","le","of","Di","A","l","S","s","L","I","T","t","D","d","i","a");
}

/**
 * Checks if the address data is sufficient.
 */
function procapi_check_address_sufficient($data) {
  // Check for Testlaan (street) 1 (house_number), Amsterdam (locality)
  if (!empty($data['street']) && !empty($data['house_number']) && !empty($data['locality'])) {
    return TRUE;
  }
  // Check for house number and postal code, for example: 1234AX 7
  // This is sufficient because we can determine the street and city
  // from the postal code.
  if (!empty($data['house_number']) && !empty($data['postal_code'])) {
    return TRUE;
  }
  return FALSE;
}


