<?php
// Profile API.
define('PROCAPI_PROFILE_CACHE_TIME', 360);
define('PROCAPI_PROFILE_ACCEPT_HEADER', 'application/vnd.procurios.profile+json; version=1');
define('PROCAPI_PROFILE_SET_PATH', '/l/profile_api/profileset');
define('PROCAPI_PROFILE_RELATION_PROFILE_PATH', '/l/profile_api/profileset/[form_id]/[relation_id]');
define('PROCAPI_REGISTRATION_SET_PATH', '/l/profile_api/registrationset');


/**
 *  Get Procurios API set.
 */
function procapi_get_set($type) {
  $set = &drupal_static(__FUNCTION__);
  if (!isset($set[$type])) {
    $set[$type] = array();
    $cache = cache_get('procapi_' . $type . '_set');
    if ($cache) {
      $set[$type] = $cache->data;
    }
    else {
      $token = procapi_get_oauth2_client_token();
      if (!empty($token)) {
        // Get set.
        $server_domain = variable_get('procapi_procurios_server_domain');
        switch ($type) {
        case 'registration':
          $path = PROCAPI_REGISTRATION_SET_PATH;
          break;

        case 'profile':
          $path = PROCAPI_PROFILE_SET_PATH;
          break;
        }
        $result = procapi_perform_request($path, 'GET', PROCAPI_PROFILE_ACCEPT_HEADER, $error);
        if (!empty($result)) {
          if (!empty($result['registrationSet'])) {
            foreach($result['registrationSet'] as $profile_form) {
              $set[$type][$profile_form['id']] = $profile_form;
            }
            cache_set('procapi_' . $type . '_set', $set[$type], 'cache', time() + PROCAPI_PROFILE_CACHE_TIME);
          }
        }
      }
    }
  }
  return $set;
}

/**
 * Get profile forms list.
 */
function procapi_get_forms($type) {
  $list = &drupal_static(__FUNCTION__);
  if (!isset($list[$type])) {
    $list[$type] = array();
    $set = procapi_get_set($type);
    foreach ($set[$type] as $form) {
      $list[$type][$form['id']] = $form['title'];
    }
  }
  return $list;
}

/**
 * Get api form schema.
 */
function procapi_get_form_schema($type) {
  $schema = &drupal_static(__FUNCTION__);
  $form_id = variable_get('procpush_api_' . $type . '_form');
  if (!isset($schema[$type][$form_id])) {
    $schema[$type][$form_id] = NULL;
    if ($cache = cache_get('procapi_' . $type . '_schema_' . $form_id)) {
      $schema[$type][$form_id] = $cache->data;
    }
    else {
      // Get set.
      $set = procapi_get_set($type);
      $path = $set[$type][$form_id]['_links']['schema']['href'];
      $method = $set[$type][$form_id]['_links']['schema']['type'];
      $result = procapi_perform_request($path, $method, PROCAPI_PROFILE_ACCEPT_HEADER, $error);
      if (!empty($result)) {
        if (is_array($result) && !empty($result['title'])) {
          $schema[$type][$form_id] = $result;
          cache_set('procapi_' . $type . '_schema_' . $form_id, $schema[$type][$form_id], 'cache', time() + PROCAPI_PROFILE_CACHE_TIME);
        }
      }
    }
  }
  return $schema;
}

/**
 * Get schema fields
 */
function procapi_get_fields($type) {
  $schema = procapi_get_form_schema($type);
  $form_id = variable_get('procpush_api_' . $type . '_form');
  if (!empty($schema[$type]) && !empty($schema[$type][$form_id])) {
    $fields = array();
    $properties = $schema[$type][$form_id]['properties'];
    $raw_fields = array();
    $key = '';
    $title = '';
    procapi_flatten_properties($properties, $raw_fields, $key, $title);
    foreach($raw_fields as $key => $title) {
      $fields[$key] = str_replace('|', ' - ', $title);
    }
    return $fields;
  }
  return FALSE;
}

/**
 * Get profile set.
 */
function procapi_profile_get_set() {
  $form_id = variable_get('procpush_api_profile_form');
  $set = procapi_get_set('profile');
  return $set['profile'][$form_id];
}

/**
 * Get profile set list.
 */
function procapi_profile_get_forms() {
  $list = procapi_get_forms('profile');
  return $list['profile'];
}

/**
 * Get profile api form schema.
 */
function procapi_profile_get_form_schema() {
  $form_id = variable_get('procpush_api_profile_form');
  $schema = procapi_get_form_schema('profile');
  return $schema['profile'][$form_id];
}

/**
 * Get profile form schema fields
 */
function procapi_profile_get_fields() {
  $fields = procapi_get_fields('profile');
  return $fields;
}

/**
 * Get relation profile.
 */
function procapi_profile_get_relation_profile($relation_id) {
  $form_id = variable_get('procpush_api_profile_form');
  $path = str_replace('[form_id]', $form_id, PROCAPI_PROFILE_RELATION_PROFILE_PATH);
  $path = str_replace('[relation_id]', $relation_id, $path);
  $result = procapi_perform_request($path, 'GET', PROCAPI_PROFILE_ACCEPT_HEADER, $error);
  if (empty($error)) {
    return $result;
  }
  return FALSE;
}

/**
 * Update user profile.
 */
function procapi_profile_update_relation($relation_id, $data_object) {
  $form_id = variable_get('procpush_api_profile_form');
  // Get profile set.
  $path = str_replace('[form_id]', $form_id, PROCAPI_PROFILE_RELATION_PROFILE_PATH);
  $path = str_replace('[relation_id]', $relation_id, $path);
  $result = procapi_perform_request($path, 'POST', PROCAPI_PROFILE_ACCEPT_HEADER, $error, NULL, $data_object);
  if (empty($error)) {
    return TRUE;
  }
  procapi_error('Fout opgetreden bij het updaten van een profiel.', $result);
  return FALSE;
}

/**
 * Get registration set.
 */
function procapi_registration_get_set() {
  $form_id = variable_get('procpush_api_registration_form');
  $set = procapi_get_set('registration');
  return $set['registration'][$form_id];
}

/**
 * Get registration set list.
 */
function procapi_registration_get_forms() {
  $list = procapi_get_forms('registration');
  return $list['registration'];
}

/**
 * Get registration api form schema.
 */
function procapi_registration_get_form_schema() {
  $form_id = variable_get('procpush_api_registration_form');
  $schema = procapi_get_form_schema('registration');
  return $schema['registration'][$form_id];
}

/**
 * Get registration form schema fields
 */
function procapi_registration_get_fields() {
  $fields = procapi_get_fields('registration');
  return $fields;
}

/**
 * Validate contact api object.
 */
function procapi_registration_validate_data_object($object) {
  // Get registration set.
  $registration_set = procapi_registration_get_set();
  $path = $registration_set['_links']['validate']['href'];
  $method = $registration_set['_links']['validate']['type'];
  $result = procapi_perform_request($path, $method, PROCAPI_PROFILE_ACCEPT_HEADER, $error, NULL, $object);
  if (empty($error)) {
    return TRUE;
  }
  return FALSE;
}

/**
 * Push contact api object.
 */
function procapi_registration_push_data_object($object) {
  // Get registration set.
  $registration_set = procapi_registration_get_set();
  $path = $registration_set['_links']['registration']['href'];
  $method = $registration_set['_links']['registration']['type'];
  $result = procapi_perform_request($path, $method, PROCAPI_PROFILE_ACCEPT_HEADER, $error, NULL, $object);
  if (empty($error)) {
    if (!empty($result)) {
      if (!empty($result['relationId'])) {
        return $result['relationId'];
      }
    }
  }
  procapi_error('Geen relatie id terug gekregen bij pushen object in registratie api.', $result);
  return FALSE;
}

/**
 * Get relation id.
 */
function procapi_registration_get_relation_id($registration_id) {
  $registration_set = procapi_registration_get_set();
  $path = $registration_set['_links']['registration']['href'] . '/' . $registration_id;
  $result = procapi_perform_request($path, 'GET', PROCAPI_PROFILE_ACCEPT_HEADER, $error);
  if (empty($error)) {
    if (!empty($result) && !empty($result['relationId'])) {
      return $result['relationId'];
    }
    else {
      procapi_error('Geen relatie id terug gekregen bij opvragen op basis registratie id.', $result);
    }
  }
  return FALSE;
}
