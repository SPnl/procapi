<?php

/**
 * Settings form.
 */
function procapi_test_form($form, &$form_state) {
  $storage = isset($form_state['storage']) ? $form_state['storage'] : array();

  $form = array();

  $form['text'] = array(
    '#markup' => '<p>Testing...</p>',
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Submit',
  );
  return $form;
}

/**
 * Form submit function.
 */
function procapi_test_form_submit($form, &$form_state) {
  $storage = &$form_state['storage'];

  //$postcode_lookup = procapi_postcode_address_lookup('3813 PK', '101');
  //dpm($postcode_lookup);


  //$selection_list = procapi_selection_get_list();
  //dpm($selection_list);
  //$selection_list = procapi_selection_get_select_list();
  //dpm($selection_list);
  //$relations = procapi_selection_get_relations(15, 1);
  //dpm($relations);

  //$token = procapi_get_oauth2_client_token();
  //dpm($token);
  //$registration_set = procapi_registration_get_set();
  //dpm($registration_set);
  //$schema = procapi_registration_get_form_schema(12);
  //dpm($schema);
  //$fields = procapi_registration_get_fields(12);
  //dpm($fields);
  
  //$profile_set = procapi_profile_get_set();
  //dpm($profile_set);
  //$schema = procapi_profile_get_form_schema(11);
  //dpm($schema);
  //$fields = procapi_profile_get_fields(11);
  //dpm($fields);
  /*
  $data = array(
    'name' => 'Pietje Puk',
    'email' => 'pietjepuk@gmail.com',
    'street' => 'Camera Obscurastraat',
    'house_number' => '101',
    'postal_code' => '3813PK',
  );
  dpm('Creating object');
  $object = procapi_create_data_object($data);
  dpm($object);
  dpm('Validating object');
  $result = procapi_proc_validate_object($object, 13, $error);
  dpm('Result');
  dpm($result);
  dpm('Error');
  dpm($error);
  dpm('Pushing object');
  $result = procapi_proc_push_object($object, 13, $error);
  dpm('Result');
  dpm($result);
  dpm('Error');
  dpm($error);
  */

  $form_state['rebuild'] = TRUE;
}

