<?php

/**
 * Settings form.
 */
function procapi_settings_form($form, &$form_state) {
  $values = isset($form_state['values']) ? $form_state['values'] : array();
  $storage = &$form_state['storage'];


  $form['#prefix'] = '<div id="procapi_ajax_form">';
  $form['#suffix'] = '</div>';

  $form['procapi_procurios_server_domain'] = array(
    '#type' => 'textfield',
    '#title' => 'Procurios server domain',
    '#default_value' => variable_get('procapi_procurios_server_domain', ''),
    '#required' => TRUE,
  );

  $form['procapi_oauth_client_id'] = array(
    '#type' => 'textfield',
    '#title' => 'Oauth client id',
    '#default_value' => variable_get('procapi_oauth_client_id', ''),
    '#required' => TRUE,
  );

  $form['procapi_oauth_client_secret'] = array(
    '#type' => 'textfield',
    '#title' => 'Oauth client secret',
    '#default_value' => variable_get('procapi_oauth_client_secret', ''),
    '#required' => TRUE,
  );

  $form['procapi_oauth_scope'] = array(
    '#type' => 'textfield',
    '#title' => 'Oauth scope',
    '#default_value' => variable_get('procapi_oauth_scope', ''),
    '#required' => TRUE,
  );

  return system_settings_form($form);
}

/**
 * Validate settings form.
 */
function procapi_settings_form_validate($form, &$form_state) {
  $values = &$form_state['values'];
  $storage = &$form_state['storage'];

  // Validate domain.
  $values['procapi_procurios_server_domain'] = preg_replace('#^https://#', '', $values['procapi_procurios_server_domain']);
  if (!filter_var('https://' . $values['procapi_procurios_server_domain'], FILTER_VALIDATE_URL)) {
    form_set_error('procapi_procurios_server_url', 'Set a valid domain (https://domain.nl).');
  }
  else {
    $url_parts = parse_url('https://' . $values['procapi_procurios_server_domain']);
    $values['procapi_procurios_server_domain'] = $url_parts['host'];
  }
}
