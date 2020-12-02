<?php

// Postcode API.
define('PROCAPI_POSTCODE_ACCEPT_HEADER', 'application/vnd.procurios.postcode+json; version=1');
define('PROCAPI_POSTCODE_LOOKUP_PATH', '/l/postcode/lookup');

function procapi_postcode_address_lookup($postcode, $number, $number_add = '', $country_code = 'NL') {
  $data = array(
    'countryCode' => $country_code,
    'postcode' => $postcode,
    'number' => $number,
    'numberAdd' => $number_add,
  );
  $result = procapi_perform_request(PROCAPI_POSTCODE_LOOKUP_PATH, 'GET', PROCAPI_POSTCODE_ACCEPT_HEADER, $error, NULL, $data);
  return $result;
}
