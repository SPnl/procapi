<?php

// Selection API.
define('PROCAPI_SELECTION_ACCEPT_HEADER', 'application/vnd.procurios.sp_custom+json; version=1');
define('PROCAPI_SELECTION_LIST_PATH', '/l/sp_api/selection');
define('PROCAPI_SELECTION_RELATION_PATH', '/l/sp_api/selection/[selection_id]/relation');
define('PROCAPI_DEPARTMENT_ACCEPT_HEADER', 'application/vnd.procurios.sp_custom+json; version=1');
define('PROCAPI_DEPARTMENT_PATH', '/l/sp_api/department');

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
        $page = 1;
        $selection_list = array();
        while ($result = procapi_perform_request(PROCAPI_SELECTION_LIST_PATH, 'GET', PROCAPI_SELECTION_ACCEPT_HEADER, $error, NULL, array('page' => $page))) {
          // Process result.
          if (!empty($result)) {
            foreach ($result as $selection_data) {
              if (!empty($selection_data) && isset($selection_data['id'])) {
                $selection_list[$selection_data['id']] = $selection_data;
              }
            }
          }
          $page++;
        }
        cache_set('procapi_selection_list', $selection_list, 'cache', time() + 360);
      }
    }
  }
  return $selection_list;
}

/**
 * Get selection select list.
 */
function procapi_selection_get_select_list() {
  $selection_list = &drupal_static(__FUNCTION__);
  if (!isset($selection_list)) {
    $selection_list = array();
    $select_list_raw = procapi_selection_get_list();
    foreach ($select_list_raw as $list_data) {
      if (isset($list_data['id']) && isset($list_data['name'])) {
        $selection_list[$list_data['id']] = $list_data['name'];
        natsort($selection_list);
      }
    }
  }
  return $selection_list;
}

/**
 * Get selection select list.
 */
function procapi_selection_get_count() {
  $selection_list = &drupal_static(__FUNCTION__);
  if (!isset($selection_list)) {
    $selection_list = array();
    $select_list_raw = procapi_selection_get_list();
    foreach ($select_list_raw as $list_data) {
      if (isset($list_data['id']) && isset($list_data['numberOfRelations'])) {
        $selection_list[$list_data['id']] = $list_data['numberOfRelations'];
      }
    }
  }
  return $selection_list;
}

/**
 * Get relations in selection.
 */
function procapi_selection_get_relations($selection_id, $page_nr = 1) {
  $relations = array();
  $selection_list = procapi_selection_get_list();
  if (!empty($selection_list[$selection_id])) {
    $path = str_replace('[selection_id]', $selection_id, PROCAPI_SELECTION_RELATION_PATH);
    $parameters = array('page' => $page_nr);
    $result = procapi_perform_request($path, 'GET', PROCAPI_SELECTION_ACCEPT_HEADER, $error, NULL, $parameters);
    if (!empty($result)) {
      $relations = $result;
    }
  }
  return $relations;
}

/**
 * Add relations to selection.
 */
function procapi_selection_add_relations($selection_id, $relations) {
  $selection_list = procapi_selection_get_list();
  if (!empty($selection_list[$selection_id])) {
    $path = str_replace('[selection_id]', $selection_id, PROCAPI_SELECTION_RELATION_PATH);
    $result = procapi_perform_request($path, 'POST', PROCAPI_SELECTION_ACCEPT_HEADER, $error, NULL, $relations);
    if (empty($error)) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
 * Add relations to selection.
 */
function procapi_selection_remove_relations($selection_id, $relations) {
  $selection_list = procapi_selection_get_list();
  if (!empty($selection_list[$selection_id])) {
    $path = str_replace('[selection_id]', $selection_id, PROCAPI_SELECTION_RELATION_PATH);
    $result = procapi_perform_request($path, 'DELETE', PROCAPI_SELECTION_ACCEPT_HEADER, $error, NULL, $relations);
    if (empty($error)) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
 * Get department info.
 */
function procapi_department_search($term) {
  $departments = array();
  $parameters = array('search' => $term);
  $result = procapi_perform_request(PROCAPI_DEPARTMENT_PATH, 'GET', PROCAPI_DEPARTMENT_ACCEPT_HEADER, $error, NULL, $parameters);
  if (!empty($result)) {
    $departments = $result;
  }
  return $departments;
}
