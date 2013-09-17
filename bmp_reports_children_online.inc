<?php

/**
 * @file
 * Provides node functions for Sibling split.
 */

function bmp_children_online($form,&$form_state) {
  $output = array();
  

  $output['content'] = array(
      'search' =>  bmp_report_children_online_search_form($form,$form_state),
      'results' =>bmp_report_children_online($form,$form_state)
      );
  
  $profile_total = bmp_reports_children_online_total_profile();
  $children_total = bmp_reports_children_online_total_children();
  $webonly_total = bmp_reports_children_online_total_webonly();
  $output['totals'] = array(
    'profiles' => array('#markup' => '<p>Profile total = '.$profile_total.'</p>'),    
    'children' => array('#markup' => '<p>Children total = '.$children_total.'</p>'),    
    'webonly' => array('#markup' => '<p>Web-only total = '.$webonly_total.'</p>'),    
  );
  
  return $output;
}


function bmp_report_children_online_search_form($form,&$form_state) {
   $form['criteria'] = array(
     '#type' => 'fieldset',
     '#title' => 'Search Criteria',
     '#attributes' => array('class' => array('container-inline')),
   );
   $form['criteria']['submitted_to_bmp'] = array(
      '#title' => t('Submitted to BMP'),
      '#type' => 'select',
      '#options' => array(
          'any' => 'Any',
          'yes' => 'Yes',
          'no' => 'No',
      ),       
   );

  $form['criteria']['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Search',
//	'#validate' => array('_split_children_validate'),
	'#submit' => array('bmp_report_children_online_search_form_submit'),
  );


  $form['criteria']['actions']['reset'] = array(
    '#type' => 'submit',
    '#value' => 'Reset',
//	'#validate' => array('_split_children_validate'),
	'#submit' => array('bmp_report_children_online_search_form_submit'),
  );
 
  return $form;
}

function bmp_report_children_online($form,&$form_state) {

  $header = array(
    array('data' => t('Title')),
    array('data' => t('No of children')),
    array('data' => t('Web only')),
    array('data' => t('Adoption and/or fostering')),
    array('data' => t('Issue')),
    array('data' => t('Ref')),
    array('data' => t('Agency region')),
    array('data' => t('Website area')),
    array('data' => t('Video clip')),
    array('data' => t('Social worker')),
    array('data' => t('SW email')),
    array('data' => t('Publish start date')),
    array('data' => t('Publish end date')),
  );


// Need to build links to the new referrals
  $select = db_select('node', 'n')
          ->extend('PagerDefault'); // this is needed to allow us to use the pager
  $select->fields('n', array('title','nid','vid'));
  $select->fields('f', array('field_feature_children_in_value',));
  $select->fields('r', array('field_child_reference_no_value',));
  $select->fields('agency_region', array('field_agency_region_tid',));
  $select->fields('wp', array('field_website_profile_tid',));
  $select->fields('v', array('field_video_fid',));
  $select->fields('a_contact', array('field_agencycontact_value',));
  $select->fields('a_email', array('field_agency_email_value',));
          

  $select->addExpression('COUNT(n.nid)', 'ncount');

  $select->condition('n.status ', 1, '=');
  $select->condition('n.type ', 'child_referral', '=');
  $select->join('field_data_field_feature_children_in','f','f.entity_id = n.nid');
  $select->join('field_data_field_child_reference_no','r','r.entity_id = n.nid');
  $select->leftJoin('field_data_field_agency_ref','aref','aref.entity_id = n.nid');
  $select->join('field_data_field_website_profile','wp','wp.entity_id = n.nid');
  $select->leftJoin('field_data_field_video','v','v.entity_id = n.nid');
  $select->leftJoin('field_data_field_agencycontact','a_contact','a_contact.entity_id = n.nid');
  $select->leftJoin('field_data_field_agency_email','a_email','a_email.entity_id = n.nid');
  $select->leftJoin('field_data_field_publish_date_start','ps','ps.entity_id = n.nid');
  $select->leftJoin('field_data_field_publish_date_end','pe','pe.entity_id = n.nid');

  $select->leftJoin('node','agency','agency.nid=aref.field_agency_ref_nid');
  $select->leftJoin('field_data_field_agency_region','agency_region','agency_region.entity_id = agency.nid');
  
  
  $query = db_select('field_data_field_sibtest','s');
  $query->groupBy('s.entity_id','s.revision_id');
  $query->addExpression('MAX(delta)', 'max_children');
  $query->fields('s',array('entity_id','revision_id'));

  $select->join($query,'s','s.entity_id = n.nid AND s.revision_id = n.vid');
  $select->fields('s',array('max_children'));
  
  $select->fields('ps',array('field_publish_date_start_value'));
  $select->fields('pe',array('field_publish_date_end_value'));
  $select->groupBy('n.title');
  
  $result = $select
      ->limit(10) // Need this to limit the number of rows
      ->orderBy('n.title')
      ->execute();

  // If there are any child referral entities linking to this one
  // then we build a list of links.

  $rows = array();
  foreach ($result as $row) {
    $row_array = array();
    $row_array['title'] = $row->title;
    $row_array['number_of_children'] = $row->max_children + 1;
    if ($row->field_feature_children_in_value == 'website_only')
      $row_array['field_feature_children_in_value'] = '✔';
    else
      $row_array['field_feature_children_in_value'] = '✖';      
    $row_array['adoption_fostering'] = bmp_reports_get_placement_type($row->nid,$row->vid);
    $row_array['issue'] = bmp_reports_get_newspaper_issues($row->nid,$row->vid);//'TBC';
    $row_array['child_ref'] = $row->field_child_reference_no_value;
    if (!empty($row->field_agency_region_tid)) {
      $term = taxonomy_term_load($row->field_agency_region_tid);
      $name = $term->name;
    } else {
      $name = '';
    }
    $row_array['agency_region'] = $name; //'TBC';
    $area = '';
    switch ($row->field_website_profile_tid) {
      case 1:
        $area = 'open';
        break;
      case 2:
        $area = 'sub';
        break;
      case 3:
        $area = 'crb';
        break;
    }
    $row_array['website_area'] = $area;
    if (!empty($row->field_video_fid)){
      $row_array['video'] = '✔';
    } else {
      $row_array['video'] = '✖';
    }
    
    if (!empty($row->field_agencycontact_value))
      $row_array['social_worker'] = $row->field_agencycontact_value;
    else
      $row_array['social_worker'] = '';
    
    if (!empty($row->field_agency_email_value))
      $row_array['sw_email'] = $row->field_agency_email_value;
    else
      $row_array['sw_email'] = '';
    
    $row_array['pub_start_date'] = $row->field_publish_date_start_value;
    $row_array['pub_end_date'] = $row->field_publish_date_end_value;
    
    //$row_array['field_child_reference_no_value'] = $row->field_child_reference_no_value;
    $rows[] = array('data' => (array) $row_array);
    
    
  }

  // Create a render array ($build) which will be themed as a table with a
  // pager.
  $output['pager_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('There are no date formats found in the db'),
  );

  // attach the pager theme
  $output['pager_pager'] = array('#theme' => 'pager');

  return $output;
}



function bmp_reports_get_newspaper_issues($nid,$vid) {
  $query = db_select('field_data_field_newspaper_issue2','np');
  $query->join('node','n','n.nid=np.field_newspaper_issue2_target_id');
  $query->fields('n',array('title'));
  $query->condition('np.entity_id',$nid,'=');
  $query->condition('np.revision_id',$vid,'=');
  
  $result = $query->execute();
  $output = '';
  foreach($result as $row) {
    if (!empty($output)) $output .= ', ';
    $output .= $row->title;
  }
  
  return $output;
}

function bmp_reports_get_placement_type($nid,$vid) {
  
  $query = db_select('field_data_field_sibtest','s');
  $query->join('field_data_field_sibplacement_type', 'f','f.entity_id = s.field_sibtest_value AND f.revision_id = s.field_sibtest_revision_id');
  $query->fields('f',array('field_sibplacement_type_value'));
  $query->condition('s.entity_id',$nid,'=');
  $query->condition('s.revision_id',$vid,'=');
  $query->condition('s.delta',0,'=');
  
  $result = $query->execute();
  
  $output = '';
  foreach($result as $row) {
    $output = $row->field_sibplacement_type_value;
  }
  
  return $output;
}

function bmp_reports_children_online_total_profile() {
  
  $query = db_select('node','n');
  $query->condition('n.status ', 1, '=');
  $query->condition('n.type ', 'child_referral', '=');
  $query->addExpression('count(n.nid)','total');
  
  $result = $query->execute();
  
  $output = '';
  foreach($result as $row) {
    $output = $row->total;
  }
  
  return $output;
}

function bmp_reports_children_online_total_children() {
  
  $query = db_select('node','n');
  $query->condition('n.status ', 1, '=');
  $query->condition('n.type ', 'child_referral', '=');
  
  
  $subquery = db_select('field_data_field_sibtest','s');
  $subquery->groupBy('s.entity_id','s.revision_id');
  $subquery->addExpression('MAX(delta)+1', 'max_children');
  $subquery->fields('s',array('entity_id','revision_id'));

  $query->join($subquery,'s','s.entity_id = n.nid AND s.revision_id = n.vid');  
  $query->addExpression('sum(max_children)','total');
  $result = $query->execute();
  
  $output = '';
  foreach($result as $row) {
    $output = $row->total;
  }
  
  return $output;
}


function bmp_reports_children_online_total_webonly() {
  
  $query = db_select('node','n');
  $query->condition('n.status ', 1, '=');
  $query->condition('n.type ', 'child_referral', '=');
  $query->join('field_data_field_feature_children_in','f','f.entity_id = n.nid');  
  $query->condition('f.field_feature_children_in_value', 'website_only', '=');
  $query->addExpression('count(n.nid)','total');
  $result = $query->execute();
  
  $output = '';
  foreach($result as $row) {
    $output = $row->total;
  }
  
  return $output;
}

function bmp_report_children_online_search_form_submit($form,&$form_state) {
  
}