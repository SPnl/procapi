<?php

/**
 * Settings form.
 */
function procapi_settings_form($form, &$form_state) {
  $values = isset($form_state['values']) ? $form_state['values'] : array();
  $storage = &$form_state['storage'];


  $form['#prefix'] = '<div id="procapi_ajax_form">';
  $form['#suffix'] = '</div>';

  $server_domain = variable_get('procapi_procurios_server_domain', '');

  $form['intro-text'] = array(
    '#markup' => '<p><strong>Wanneer je een bestaande verbinding wijzigt moet je de Drupal caches legen, en opnieuw inloggen voor deze actief wordt.</strong></p>',
  );

  $form['procapi_procurios_server_domain'] = array(
    '#type' => 'textfield',
    '#title' => 'Procurios server domain',
    '#default_value' => $server_domain,
    '#required' => TRUE,
    '#ajax' => array(
      'wrapper' => 'procapi_ajax_form',
      'callback' => 'procapi_ajax_form',
    ),
  );

  $client_id = variable_get('procapi_oauth_client_id', '');
  $form['procapi_oauth_client_id'] = array(
    '#type' => 'textfield',
    '#title' => 'Oauth client id',
    '#default_value' => $client_id,
    '#required' => TRUE,
    '#ajax' => array(
      'wrapper' => 'procapi_ajax_form',
      'callback' => 'procapi_ajax_form',
    ),
  );

  $client_secret = variable_get('procapi_oauth_client_secret', '');
  $form['procapi_oauth_client_secret'] = array(
    '#type' => 'textfield',
    '#title' => 'Oauth client secret',
    '#default_value' => $client_secret,
    '#required' => TRUE,
    '#ajax' => array(
      'wrapper' => 'procapi_ajax_form',
      'callback' => 'procapi_ajax_form',
    ),
  );

  $scope_list = procapi_get_scope_list();
  $scopes = variable_get('procapi_oauth_scopes');
  $form['procapi_oauth_scopes'] = array(
    '#type' => 'checkboxes',
    '#multiple' => TRUE,
    '#title' => 'Selecteer de gewenste oauth scope(s)',
    '#required' => TRUE,
    '#options' => $scope_list,
    '#default_value' => $scopes,
    '#ajax' => array(
      'wrapper' => 'procapi_ajax_form',
      'callback' => 'procapi_ajax_form',
    ),
  );

  if (!empty($server_domain) && !empty($client_id) && !empty($client_secret) && !empty($scopes)) {
    // Check if oauth2 token can be retrieved.
    $token = procapi_get_oauth2_client_token();
    if (empty($token)) {
      drupal_set_message('Er kon geen verbinding gemaakt worden met de Procurios API.', 'warning');
    }
    else {
      if (!empty($scopes['profile/profile'])) {
        // Get profile form from procurios.
        $profile_form_id = variable_get('procapi_profile_form');
        $api_profile_form_list = procapi_profile_get_forms();
        $form['procapi_profile_form'] = array(
          '#type' => 'select',
          '#title' => 'Procurios profile API form',
          '#options' => $api_profile_form_list,
          '#default_value' => $profile_form_id,
          '#required' => TRUE,
          '#ajax' => array(
            'wrapper' => 'procapi_ajax_form',
            'callback' => 'procapi_ajax_form',
          ),
        );
      }

      if (!empty($scopes['profile/registration'])) {
        // Get registration form from procurios.
        $registration_form_id = variable_get('procapi_registration_form');
        $api_registration_form_list = procapi_registration_get_forms();
        $form['procapi_registration_form'] = array(
          '#type' => 'select',
          '#title' => 'Procurios registration API form',
          '#options' => $api_registration_form_list,
          '#default_value' => $registration_form_id,
          '#required' => TRUE,
          '#ajax' => array(
            'wrapper' => 'procapi_ajax_form',
            'callback' => 'procapi_ajax_form',
          ),
        );
      }
    }
  }

  return system_settings_form($form);
}

/**
 * Validate settings form.
 */
function procapi_settings_form_validate($form, &$form_state) {
  $values = &$form_state['values'];

  // Validate domain.
  $values['procapi_procurios_server_domain'] = preg_replace('#^https://#', '', $values['procapi_procurios_server_domain']);
  if (!filter_var('https://' . $values['procapi_procurios_server_domain'], FILTER_VALIDATE_URL)) {
    form_set_error('procapi_procurios_server_url', 'Set a valid domain (https://domain.nl).');
  }
  else {
    $url_parts = parse_url('https://' . $values['procapi_procurios_server_domain']);
    $values['procapi_procurios_server_domain'] = $url_parts['host'];
  }

  // Store values.
  foreach ($values as $key => $value) {
    if (strpos($key, 'procapi_') === 0) {
      variable_set($key, $value);
    }
  }

  // Store oauth scope.
  foreach ($values['procapi_oauth_scopes'] as $scope) {
    if (!empty($scope)) {
      $selected_scopes[] = $scope;
    }
  }
  $procapi_oauth_scope = implode($selected_scopes, ' ');
  variable_set('procapi_oauth_scope', $procapi_oauth_scope);
}

/**
 * Ajax return form.
 */
function procapi_ajax_form($form, &$form_state) {
 return $form;
}
