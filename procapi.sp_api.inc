<?php

// Selection API.
define("PROCAPI_API_SELECTION_ACCEPT_HEADER", 'application/vnd.procurios.sp_custom+json; version=1');
define("PROCAPI_API_SELECTION_LIST_PATH", '/l/sp_api/selection');

/**
 * Get selection list.
 */
function procapi_selection_get_list() {
  $selection_list = &drupal_static(__FUNCTION__);
  if (!isset($selection_list)) {
    $selection_list = array();
    if ($cache = cache_get('procapi_selection_list')) {
      $selection_list = $cache->data;
    }
    else {
      $token = procapi_get_oauth2_client_token();
      if (!empty($token)) {
        // Get registration set.
        $server_domain = variable_get('procapi_procurios_server_domain');
        $result = procapi_perform_request(PROCAPI_API_SELECTION_LIST_PATH, 'GET', PROCAPI_API_SELECTION_ACCEPT_HEADER, $error);
        if (!empty($result)) {
          if (!empty($result['registrationSet'])) {
            foreach($result['registrationSet'] as $registration_form) {
              $selection_list[$registration_form['id']] = $registration_form;
            }
            cache_set('procapi_selection_list', $selection_list, 'cache', time() + 360);
          }
        }
      }
    }
  }
  return $selection_list;
}


