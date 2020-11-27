<?php
// Profile API.
define('PROCAPI_PROFILE_ACCEPT_HEADER', 'application/vnd.procurios.profile+json; version=1');
define('PROCAPI_PROFILE_SET_PATH', '/l/profile_api/profileset');
define('PROCAPI_REGISTRATION_SET_PATH', '/l/profile_api/registrationset');


/**
 * Get profile set.
 */
function procapi_profile_get_set() {
  $profile_set = &drupal_static(__FUNCTION__);
  if (!isset($profile_set)) {
    $profile_set = array();
    if ($cache = cache_get('procapi_profile_set')) {
      $profile_set = $cache->data;
    }
    else {
      $token = procapi_get_oauth2_client_token();
      if (!empty($token)) {
        // Get profile set.
        $server_domain = variable_get('procapi_procurios_server_domain');
        $result = procapi_perform_request(PROCAPI_PROFILE_SET_PATH, 'GET', PROCAPI_PROFILE_ACCEPT_HEADER, $error);
        if (!empty($result)) {
          if (!empty($result['registrationSet'])) {
            foreach($result['registrationSet'] as $profile_form) {
              $profile_set[$profile_form['id']] = $profile_form;
            }
            cache_set('procapi_profile_set', $profile_set, 'cache', time() + 360);
          }
        }
      }
    }
  }
  return $profile_set;
}

/**
 * Get profile set list.
 */
function procapi_profile_get_forms() {
  $list = &drupal_static(__FUNCTION__);
  if (!isset($list)) {
    $list = array();
    $profile_set = procapi_profile_get_set();
    foreach ($profile_set as $profile_form) {
      $list[$profile_form['id']] = $profile_form['title'];
    }
  }
  return $list;
}

/**
 * Get profile api form schema.
 */
function procapi_profile_get_form_schema($form_id) {
  $schema = &drupal_static(__FUNCTION__);
  if (!isset($schema[$form_id])) {
    $schema[$form_id] = NULL;
    if ($cache = cache_get('procapi_profile_schema_' . $form_id)) {
      $schema[$form_id] = $cache->data;
    }
    else {
      // Get profile set.
      $profile_set = procapi_profile_get_set();
      $path = $profile_set[$form_id]['_links']['schema']['href'];
      $method = $profile_set[$form_id]['_links']['schema']['type'];
      $result = procapi_perform_request($path, $method, PROCAPI_PROFILE_ACCEPT_HEADER, $error);
      if (!empty($result)) {
        if (is_array($result) && !empty($result['title'])) {
          $schema[$form_id] = $result;
          cache_set('procapi_profile_schema_' . $form_id, $schema[$form_id], 'cache', time() + 360);
        }
      }
    }
  }
  return $schema;
}

/**
 * Get profile form schema fields
 */
function procapi_profile_get_fields($form_id) {
  $schema = procapi_profile_get_form_schema($form_id);
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
 * Get registration set.
 */
function procapi_registration_get_set() {
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
        $result = procapi_perform_request(PROCAPI_REGISTRATION_SET_PATH, 'GET', PROCAPI_PROFILE_ACCEPT_HEADER, $error);
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
 * Get registration set list.
 */
function procapi_registration_get_forms() {
  $list = &drupal_static(__FUNCTION__);
  if (!isset($list)) {
    $list = array();
    $registration_set = procapi_registration_get_set();
    foreach ($registration_set as $registration_form) {
      $list[$registration_form['id']] = $registration_form['title'];
    }
  }
  return $list;
}

/**
 * Get registration api form schema.
 */
function procapi_registration_get_form_schema($form_id) {
  $schema = &drupal_static(__FUNCTION__);
  if (!isset($schema[$form_id])) {
    $schema[$form_id] = NULL;
    if ($cache = cache_get('procapi_registration_schema_' . $form_id)) {
      $schema[$form_id] = $cache->data;
    }
    else {
      // Get registration set.
      $registration_set = procapi_registration_get_set();
      $path = $registration_set[$form_id]['_links']['schema']['href'];
      $method = $registration_set[$form_id]['_links']['schema']['type'];
      $result = procapi_perform_request($path, $method, PROCAPI_PROFILE_ACCEPT_HEADER, $error);
      if (!empty($result)) {
        if (is_array($result) && !empty($result['title'])) {
          $schema[$form_id] = $result;
          cache_set('procapi_registration_schema_' . $form_id, $schema[$form_id], 'cache', time() + 360);
        }
      }
    }
  }
  return $schema;
}

/**
 * Get registration form schema fields
 */
function procapi_registration_get_fields($form_id) {
  $schema = procapi_registration_get_form_schema($form_id);
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
 * Validate contact api object.
 */
function procapi_registration_validate_object($object, $form_id, &$error) {
  // Get registration set.
  $registration_set = procapi_registration_get_set();
  $path = $registration_set[$form_id]['_links']['validate']['href'];
  $method = $registration_set[$form_id]['_links']['validate']['type'];
  $result = procapi_perform_request($path, $method, PROCAPI_PROFILE_ACCEPT_HEADER, $error, NULL, json_encode($object));
  return $result;
}

/**
 * Push contact api object.
 */
function procapi_registration_push_object($object, $form_id, &$error) {
  // Get registration set.
  $registration_set = procapi_registration_get_set();
  $path = $registration_set[$form_id]['_links']['registration']['href'];
  $method = $registration_set[$form_id]['_links']['registration']['type'];
  $result = procapi_perform_request($path, $method, PROCAPI_PROFILE_ACCEPT_HEADER, $error, NULL, json_encode($object));
  return $result;
}
