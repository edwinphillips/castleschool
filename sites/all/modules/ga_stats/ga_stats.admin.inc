<?php

/**
 * Callback for GA Stats authentication form.
 */
function ga_stats_auth_settings() {
  $form = array();

  $private_path = variable_get('file_private_path', FALSE);
  if (empty($private_path)) {
    drupal_set_message(t('Your need to set the private file system path. Visit the !filesystem to configure it.',
        array('!filesystem' => l(t('File system config page'), 'admin/config/media/file-system'))), 'error');
      $form['#disabled'] = TRUE;
  }

  $form['ga_stats_login'] = array(
    '#type' => 'fieldset',
    '#title' => t('Google Analytics Login Information'),
    '#collapsible' => TRUE,
    '#collapsed' => ga_stats_is_ready(),
  );

  $form['ga_stats_login']['ga_stats_app_name'] = array(
    '#type' => 'textfield',
    '#title' => t('App Name'),
    '#description' => t('The name of your app on google'),
    '#default_value' => variable_get('ga_stats_app_name', '')
  );

  $form['ga_stats_login']['ga_stats_email'] = array(
    '#type' => 'textfield',
    '#title' => t('Google App Email'),
    '#description' => t('The email address which was provided when you created App API credentials'),
    '#default_value' => variable_get('ga_stats_email', '')
  );

  $private_key_path = variable_get('ga_stats_private_key_p12');
  $form['ga_stats_login']['private_key'] = array(
    '#type' => 'file',
    '#title' => t('Private Key'),
    '#description' => empty($private_key_path) ? 'No file uploaded. Please upload. (Allowed extensions: p12)' : 'The key file is uploaded. Uploading another file will override the existing one. (Allowed extensions: p12)',
    '#upload_validators'  => array('file_validate_extensions' => array('p12')),
  );
  $form['#submit'][] = 'ga_stats_custom_form_submit';

  return system_settings_form($form);
}

/**
 * Custom form submit callback.
 */
function ga_stats_custom_form_submit($form, &$form_state) {
  $validators = array('file_validate_extensions' => array('p12'));
  if ($file = file_save_upload('private_key', $validators, 'private://')) {
    // Renaming the file, so that every uploaded key would override the existing key.
    $file = file_move($file, 'private://ga_stats_p12_key.p12', FILE_EXISTS_REPLACE);
    // Make the file permanent.
    $file->status = 1;
    file_save($file);
    // Save the file uri.
    variable_set('ga_stats_private_key_p12', $file->uri);
  }
}

/**
 * Callback for the GA Stats admin form.
 */
function ga_stats_admin_settings() {
  $form = array();

  $form['ga_stats_accounts'] = array(
    '#type' => 'fieldset',
    '#title' => t('Google Analytics Tracking Accounts'),
    '#collapsible' => TRUE,
  );

  if (!ga_stats_is_ready()) {
    drupal_set_message(t('Your Google Analytics account credentials are not set. Visit the !authentication to configure them.',
      array('!authentication' => l(t('authentication tab'), 'admin/config/services/ga_stats/auth'))), 'error');
    $form['#disabled'] = TRUE;
    $accounts = FALSE;
  }
  else {
    require_once 'includes/ga.inc';
    // @todo expand on the error cases here.
    $accounts = ga_stats_ga_get_accounts(ga_stats_get_client());
  }

  $options = array();
  if (!empty($accounts) && is_array($accounts)) {
    foreach ($accounts as $id => $value) {
      $acc = $value->getProperties();
      foreach ($acc as $key => $val) {
        foreach($val['profiles'] as $k => $v) {
          $options[$v['id']] = $v['name'];
        }
      }
    }

    $form['ga_stats_accounts']['ga_stats_profile'] = array(
      '#type' => 'select',
      '#title' => t('Google Analytics Profile to Use'),
      '#description' => t('The Google Analytics profile from which to retrieve statistics'),
      '#options' => $options,
      '#default_value' => variable_get('ga_stats_profile', ''),
    );
  }
  else {
    $form['ga_stats_accounts']['ga_stats_profile'] = array(
      '#type' => 'markup',
      '#markup' => '<div class="messages warning">'
        . t('Cannot select a tracking account until your Google Analytics !configured.', array('!configured' => l(t('account is configured'), 'admin/config/services/ga_stats/auth')))
        . '</div>'
    );
  }

  $form['enabled_stats'] = array(
    '#type' => 'fieldset',
    '#title' => t('Enabled Statistics'),
    '#description' => t('Make sure to clear the Drupal cache after changing this setting in order to inform Views of the new settings. <br/><em><b>WARNING:</b> Do not disable a setting which is currently in use in Views.</em>')
  );

  $form['enabled_stats']['ga_stats_enabled_metrics'] = array(
    '#type' => 'checkboxes',
    '#default_value' => variable_get('ga_stats_enabled_metrics', array('pageviews')),
    '#options' => ga_stats_ga_metrics(TRUE),
    '#title' => t('Metrics'),
    '#description' => t('The metrics that will be available from Google Analytics in Views.'),
  );

  $form['enabled_stats']['ga_stats_enabled_timeframes'] = array(
    '#type' => 'checkboxes',
    '#default_value' => variable_get('ga_stats_enabled_timeframes', array('today', 'month')),
    '#options' => ga_stats_ga_timeframes(TRUE, TRUE),
    '#title' => t('Time Frames'),
    '#description' => t('The timeframes that will be available from Google Analytics in Views.'),
  );

  $form['ga_stats_max_results'] = array(
    '#type' => 'textfield',
    '#title' => t('Max Results per Metric/Timeframe'),
    '#description' => t('The max results that a call (a metric/timeframe combination) can return. MUST be a number.'),
    '#default_value' => variable_get('ga_stats_max_results', '100'),
  );

  if (variable_get('ga_stats_profile', FALSE)) {
    $form['actions']['ga_stats_update'] = array(
      '#type' => 'button',
      '#value' => t('Update Counts'),
      '#weight' => 20,
      '#executes_submit_callback' => TRUE,
      '#submit' => array('ga_stats_update_counts'),
    );
  }

  return system_settings_form($form);
}

/**
 * Submit callback to update the statistics data.
 */
function ga_stats_update_counts_submit($form, &$form_state) {
  if (ga_stats_update_counts()) {
    drupal_set_message(t('Successfully retrieved Analytics data.'));
    drupal_set_message(t('The next scheduled retrieval is on !date', array(
      '!date' => date('r', ga_stats_data_expiration_date()),
    )));
  }
}
