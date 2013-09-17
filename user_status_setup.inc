<?php

/**
 * @file
 * Provides functions for setting up the site wide status.
 */


/**
 * Author : Stephen Blinkhorne
 *  stephen.blinkhorne@talquin.com
 * 
 * Created 3rd July 2013
 * 
 */

/**
 * Show the form to allow the user to change the settings.
 *
 * @param 
 *   none
 *
 * @return
 *   Form array to be rendered
 */
function user_status_settings() {

  // Give some information on the setting up process.
  // Might need to add code to prevent or warn user if they decide to change it
  $message  = '<h1>User status set up.</h1> ';
  $message .= 'Please select the default status for the site.<br>';

  $output['comment'] = array(
        '#markup' => t($message),
        '#weight' => 1,
      );

  $user_status_setting_form = drupal_get_form('user_status_setting_form');

  $output['form'] = array(
      '#type' => 'item',
      '#markup' => drupal_render($user_status_setting_form),
      '#weight' => 5,
  );

  return $output;
}

/**
 * Form to allow the user to set the global status.
 *
 * @param 
 *
 * @return
 *   Form array.
 */
function user_status_setting_form($form, &$form_state) {

  $form = array();

  // Provide the list of options.
  $form['setting'] = array(
      '#type' => 'select',
      '#title' => t('User status'),
      '#options' => user_status_get_options(),
      '#weight' => 2,      
  );
  
  $form['action'] = array (
      '#type' => 'container',
      '#prefix' => '<div id="tab-buttons">',
      '#suffix' => '</div>',
      '#weight' => 10,      
  );

  // Add the submit button so we can process the response.
  $form['action']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
      '#submit' => array('user_status_setting_submit'),
      
  );

  return $form;
}

/**
 * Process the response from the form submit.
 * 
 * We will store the user's selection in the database.
 *
 * @param 
 * 
 * @return
 *   Form array.
 */
function user_status_setting_submit($form,&$form_state) {

  // Save the global response to the database
  if (!empty($form_state['values']['setting'])) {
    $return_value = _user_status_default_insert($form_state['values']['setting']);

    if ($return_value <> NULL) {
      $status_name = _get_user_status($form_state['values']['setting']);
      drupal_set_message('Your user status <strong>' . $status_name . '</strong> has been saved');
    }
  }
}

/**
 * Insert the status change into the database.
 *
 * @param 
 *   $entry - The ID of hte status we want to use site wide
 *
 * @return
 *   Form array.
 */
function _user_status_default_insert($entry) {
  if (empty($entry)) {
    return;
  }
  
  $fields = array(
    'sid'    => $entry,
  );
  
  $return_value = NULL;
  try {
    $return_value = db_insert('user_status_default')
                    ->fields($fields)
                    ->execute();
  }
  catch (Exception $e) {
    drupal_set_message(t('db_insert failed. Message = %message, query= %query',
      array('%message' => $e->getMessage(), '%query' => $e->query_string)), 'error');
  }
  return $return_value;
}

/*
 * Get list of option for the user status
 */
function user_status_get_options() {
  $query = db_select('user_status','us');
  $query->fields('us',array('sid','name'));
  
  $result = $query->execute();
  $options = array();
  
  foreach($result as $row) {
    $options[$row->sid] = $row->name;
  }
  
  return $options;
}

/*
 * Get the name of the user status
 */
function _get_user_status($status_id) {
  $query = db_select('user_status','us');
  $query->fields('us',array('sid','name'));
  $query->condition('us.sid',$status_id,'=');
  $query->range(0,1);
  $result = $query->execute();
  $name = '';
  
  foreach($result as $row) {
    $name = $row->name;
  }

  return $name;
}

