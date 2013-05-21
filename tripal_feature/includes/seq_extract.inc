<?php
/*
 * 
 */
function tripal_feature_seq_extract_page() {
  if ($_SESSION['tripal_feature_seq_extract']){   
    $genus      = $_SESSION['tripal_feature_seq_extract']['genus'];
    $species    = $_SESSION['tripal_feature_seq_extract']['species'];
    $analysis   = $_SESSION['tripal_feature_seq_extract']['analysis'];
    $ftypes     = $_SESSION['tripal_feature_seq_extract']['ftypes'];
    $fname      = $_SESSION['tripal_feature_seq_extract']['fname'];
    $upstream   = $_SESSION['tripal_feature_seq_extract']['upstream'];
    $downstream = $_SESSION['tripal_feature_seq_extract']['downstream'];
    $format     = $_SESSION['tripal_feature_seq_extract']['format'];
    $use_parent = $_SESSION['tripal_feature_seq_extract']['use_parent'];
    $aggregate  = $_SESSION['tripal_feature_seq_extract']['aggregate'];
    $agg_types  = $_SESSION['tripal_feature_seq_extract']['agg_types'];
     
    if ($format == 'fasta_html') {
      drupal_set_header('Content-Type: text/html');
    }
    else {
      drupal_set_header('Content-Type: text/plain');
    }
    tripal_feature_seq_extract_get_features(NULL, $genus, $species, $analysis, 
      $ftypes, $fname, $upstream, $downstream, $format, $use_parent, $aggregate, $agg_types);
      
    unset($_SESSION['tripal_feature_seq_extract']);
    return;
  }
 
  
  // generate the search form 
  $output .= '';
  if (user_access('access administration pages')) { 
    $output .= "
      <div class=\"tripal-no-results\">
        <p>Administrators, the " . l('organism_feature_count', 'admin/tripal/mviews') . " and 
        " . l('analysis_organism', 'admin/tripal/mviews') . " materialized
        views must be populated before using this form.  Those views should be re-populated 
        when new data is added.</p>         
      </div>
    ";
  }
  $output .= drupal_get_form('tripal_feature_seq_extract_form');  
  return $output;
}
/*
 * 
 */
function theme_tripal_feature_seq_extract_form($form) {
  $rows = array(
    0 => array(
      drupal_render($form['genus']),
      drupal_render($form['species']) ,
      drupal_render($form['ftypes']) ,
    ),
    1 => array(
      array('data' => drupal_render($form['analysis']), 'colspan' => 2),
      drupal_render($form['format']),    
    ),
    2 => array(     
      array('data' =>  drupal_render($form['fname']), 'colspan' => 2),
      drupal_render($form['upstream']) . drupal_render($form['downstream']) ,
    ),
    3 => array(
      array(
        'data' =>  drupal_render($form['advanced']),
        'colspan' => 3,
      ),    
    ),
    4 => array(
      array(
        'data' =>  drupal_render($form['retrieve_btn']),
        'colspan' => 3,
      ),    
    ),
  );
  $headers = array();
  $table = theme('table', $headers, $rows, array('id' => 'tripal-feature-seq-extract-form-table', 'border' => '0'));   
  
  $markup = ''; 
  $markup .= $table;
  $form['criteria'] = array(
    '#type' => 'markup',
    '#value' =>  $markup,
    '#weight' => -10,
  );
  return drupal_render($form);
}
/**
*
 */
function tripal_feature_seq_extract_form(&$form_state = NULL) {   
  tripal_core_ahah_init_form();
  
  // get defaults
  $dgenus      = $form_state['values']['genus'];
  $dspecies    = $form_state['values']['species'];
  $danalysis   = $form_state['values']['analysis'];
  $dftypes     = $form_state['values']['ftypes'];
  $dfname      = $form_state['values']['fname'];
  $dupstream   = $form_state['values']['upstream'];
  $ddownstream = $form_state['values']['downstream'];
  $dformat     = $form_state['values']['format'] ? $form_state['values']['format'] : 'fasta_txt';
  $duse_parent = $form_state['values']['use_parent'];
  $daggregate  = $form_state['values']['aggregate'];
  $dagg_types  = $form_state['values']['agg_types'];

  $sql = "
    SELECT DISTINCT genus 
    FROM {organism} 
    ORDER BY genus
  ";
  $results = chado_query($sql);
  $genus = array();
  $genus[] = '';
  while ($organism = db_fetch_object($results)) {
    $genus[$organism->genus] = $organism->genus;  
  }
  $form['genus'] = array(
    '#title'         => t('Genus'),
    '#type'          => 'select',
    '#options'       => $genus, 
    '#default_value' => $dgenus,    
    '#multiple'      => FALSE,
    '#description'   => t('The organism\'s genus. If specified, features for all organism with this genus will be retrieved.'),
    '#ahah' => array(
      'path'    => 'seq_extract/set_genus',
      'wrapper' => 'tripal-feature-seq-extract-form-table',
      'event'   => 'change',
      'method'  => 'replace',
    ), 
  );
  
  $species = array();  
  if ($dgenus) {
    $sql = "
      SELECT DISTINCT species 
      FROM {organism} 
      WHERE genus = '%s'
      ORDER BY species
    ";
    $results = chado_query($sql, $dgenus);
    $species[] = '';
    while ($organism = db_fetch_object($results)) {
      $species[$organism->species] = $organism->species;  
    }
  }  
  $form['species'] = array(
    '#title'         => t('Species'),
    '#type'          => 'select',
    '#options'       => $species,
    '#default_value' => $dspecies,     
    '#multiple'      => FALSE,
    '#description'   => t('The organism\'s species name. If specified, features for all organisms with this species will be retrieved.  Please first select a genus'),
    '#ahah' => array(
      'path'    => 'seq_extract/set_species',
      'wrapper' => 'tripal-feature-seq-extract-form-table',
      'event'   => 'change',
      'method'  => 'replace',
    ), 
  );

  $analyses = array();
  if ($dgenus) {
    $sql = "
      SELECT DISTINCT A.analysis_id, A.name
      FROM {analysis_organism} AO 
        INNER JOIN {analysis} A ON A.analysis_id = AO.analysis_id
        INNER JOIN {organism} O ON O.organism_id = AO.organism_id
      WHERE O.genus = '%s'
    ";
    $args = array();
    $args[] = $dgenus;
    if ($dspecies) {
      $sql .= " AND O.species = '%s' ";
      $args[] = $dspecies;
    }
    $results = chado_query($sql, $args);  
    $analyses[] = '';
    while ($analysis = db_fetch_object($results)) {
      $analyses[$analysis->analysis_id] = $analysis->name;  
    }
  }
  $form['analysis'] = array(
    '#title'         => t('Source'),
    '#type'          => 'select',
    '#options'       => $analyses, 
    '#default_value' => $danalysis,    
    '#multiple'      => FALSE,
    '#description'  => t('The feature source. If specified, only features derived or part of the selected source will be retrieved.'),
    '#ahah' => array(
      'path'    => 'seq_extract/set_source',
      'wrapper' => 'tripal-feature-seq-extract-form-table',
      'event'   => 'change',
      'method'  => 'replace',
    ), 
  );
  
  $ftypes = array();
  if ($dgenus) {  
    $sql = "
      SELECT DISTINCT OFC.cvterm_id, OFC.feature_type
      FROM {organism_feature_count} OFC 
      WHERE OFC.genus = '%s'
    ";
    $args = array();
    $args[] = $dgenus;
    if ($dspecies) {
      $sql .= " AND OFC.species = '%s'";
      $args[] = $dspecies;
    }
    $results = chado_query($sql, $args);
    
    $ftypes[] = '';
    while ($type = db_fetch_object($results)) {
      $ftypes[$type->cvterm_id] = $type->feature_type;  
    }
  }
  $form['ftypes'] = array(
    '#title'         => t('Feature Types'),
    '#type'          => 'select',
    '#options'       => $ftypes,     
    '#multiple'      => FALSE,
    '#default_value' => $dftypes,
    '#description'   => t('The type of feature to retrieve (e.g. mRNA). All features that match this type will be retrieved.'),
  );
  
  $form['fname'] = array(
    '#title'         => t('Feature Name'),
    '#type'          => 'textarea',
    '#default_value' => $dfname,
    '#description'   => t('The names of the features to retrieve. Separate each with a space. Leave blank to retrieve all features matching other criteria.'),
    '#rows'          => 8
  );
  $form['upstream'] = array(
    '#title'         => t('Upstream Bases'),
    '#type'          => 'textfield',
    '#description'   => t('An numeric value specifying the number of upstream bases to include.'),
    '#default_value' => $dupstream,
    '#size'          => 5,
  );
  $form['downstream'] = array(
    '#title'         => t('Downstream Bases'),
    '#type'          => 'textfield',
    '#description'   => t('An numeric value specifying the number of downstream bases to incldue.'),
    '#default_value' => $ddownstream,
    '#size'          => 5,
  );
  $form['format'] = array(
    '#title'         => t('Output Format'),
    '#type'          => 'select',
    '#default_value' => $dformat,
    '#options'       => array(
      'fasta_html'  => 'FASTA (in browser)',
      'fasta_txt'   => 'FASTA (download)',
    ),
  );
  $form['advanced'] = array(
    '#type' => 'fieldset',
    '#title' => 'Advanced',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE
  );
  
  $form['advanced']['use_parent'] = array(
    '#title'         => t('Use Parent'),
    '#type'          => 'checkbox',
    '#default_value' => $duse_parent,
    '#description'   => t('Check this box to retrieve the sequence from the parent in an alignment rather than the feature itself. This is useful if the same feature is aligned to multiple parents and you would like to retrieve the underlying sequence from each parent.'),
  );
  $form['advanced']['aggregate'] = array(
    '#title'         => t('Aggregate'),
    '#type'          => 'checkbox',
    '#default_value' => $daggregate,
    '#description'   => t('Check this box to aggregate sub features into a single sequence.  This is useful, for example, for obtaining CDS sequence from an mRNA. Rather than retrieve the mRNA sequence, the sub features of the mRNA will be aggregated and that will be returned.')
  );
  $form['advanced']['agg_types'] = array(
    '#title'         => t('Types to aggregate'),
    '#type'          => 'textarea',
    '#default_value' => $dagg_types,
    '#description'   => t('Set this argument to the type of children to aggregate.  This is useful in the case where a gene has exons, CDSs and UTRs.  In this case, you may only want to aggregate CDSs and exclude exons.  If you want to aggregate both CDSs and UTRs you could specify both.')
  );
  $form['retrieve_btn'] = array(
    '#type' => 'submit',
    '#value' => 'Retrieve'
  );
  return $form;
}

/**
 * 
 */
function tripal_feature_seq_extract_set_genus() {
  $status = TRUE;

  // prepare and render the form
  $form = tripal_core_ahah_prepare_form();   
  $data = theme('tripal_feature_seq_extract_form', $form);  

  // bind javascript events to the new objects that will be returned 
  // so that AHAH enabled elements will work.
  $settings = tripal_core_ahah_bind_events();

  // return the updated JSON
  drupal_json(
    array(
      'status'   => $status, 
      'data'     => $data,
      'settings' => $settings,
    )  
  );
}
/**
 * 
 */
function tripal_feature_seq_extract_set_species() {
  $status = TRUE;

  // prepare and render the form
  $form = tripal_core_ahah_prepare_form();   
  $data = theme('tripal_feature_seq_extract_form', $form); 

  // bind javascript events to the new objects that will be returned 
  // so that AHAH enabled elements will work.
  $settings = tripal_core_ahah_bind_events();

  // return the updated JSON
  drupal_json(
    array(
      'status'   => $status, 
      'data'     => $data,
      'settings' => $settings,
    )  
  );
}
/**
 * 
 */
function tripal_feature_seq_extract_set_source() {
  $status = TRUE;

  // prepare and render the form
  $form = tripal_core_ahah_prepare_form();   
  $data = theme('tripal_feature_seq_extract_form', $form); 

  // bind javascript events to the new objects that will be returned 
  // so that AHAH enabled elements will work.
  $settings = tripal_core_ahah_bind_events();

  // return the updated JSON
  drupal_json(
    array(
      'status'   => $status, 
      'data'     => $data,
      'settings' => $settings,
    )  
  );
}

/**
 * 
 */
function tripal_feature_seq_extract_form_validate($form, &$form_state) {
  $genus      = $form_state['values']['genus'];
  $species    = $form_state['values']['species'];
  $analysis   = $form_state['values']['analysis'];
  $ftypes     = $form_state['values']['ftypes'];
  $fname      = $form_state['values']['fname'];
  $upstream   = $form_state['values']['upstream'];
  $downstream = $form_state['values']['downstream'];
  $format     = $form_state['values']['format'];
  $use_parent = $form_state['values']['use_parent'];
  $aggregate  = $form_state['values']['aggregate'];
  $agg_types  = $form_state['values']['agg_types'];  
  
  if ($upstream and !preg_match('/^\d+$/', $upstream)) {
    form_set_error('upstream', 'Please enter a positive numeric value for the upstream bases');
  }
   if ($downstream and !preg_match('/^\d+$/', $downstream)) {
    form_set_error('downstream', 'Please enter a positive numeric value for the downstream bases');
  }
}

/**
 * 
 */
function tripal_feature_seq_extract_form_submit($form, &$form_state) {
  $genus      = $form_state['values']['genus'];
  $species    = $form_state['values']['species'];
  $analysis   = $form_state['values']['analysis'];
  $ftypes     = $form_state['values']['ftypes'];
  $fname      = $form_state['values']['fname'];
  $upstream   = $form_state['values']['upstream'];
  $downstream = $form_state['values']['downstream'];
  $format     = $form_state['values']['format'];
  $use_parent = $form_state['values']['use_parent'];
  $aggregate  = $form_state['values']['aggregate'];
  $agg_types  = $form_state['values']['agg_types'];
  
  $_SESSION['tripal_feature_seq_extract']['genus']      = $genus;
  $_SESSION['tripal_feature_seq_extract']['species']    = $species;
  $_SESSION['tripal_feature_seq_extract']['analysis']   = $analysis;
  $_SESSION['tripal_feature_seq_extract']['ftypes']     = $ftypes;
  $_SESSION['tripal_feature_seq_extract']['fname']      = $fname;
  $_SESSION['tripal_feature_seq_extract']['upstream']   = $upstream;
  $_SESSION['tripal_feature_seq_extract']['downstream'] = $downstream;
  $_SESSION['tripal_feature_seq_extract']['format']     = $format;
  $_SESSION['tripal_feature_seq_extract']['use_parent'] = $use_parent;
  $_SESSION['tripal_feature_seq_extract']['aggregate']  = $aggregate;
  $_SESSION['tripal_feature_seq_extract']['agg_types']  = $agg_types;
}

/*
 * 
 */
function tripal_feature_seq_extract_get_features($org_commonname, $genus, $species, $analysis_name, 
  $type, $feature_name, $upstream, $downstream, $output_format, $derive_from_parent, $aggregate, $child) {
    
  $sub_features = explode(',', $child);
    
  if (!$output_format) {
     $output_format = 'fasta_txt';
  }
  
  if (!$type and !$feature_name and !$org_commonname) {
     print "Please provide a type, feature name or organism common name\n";
     return;
  }

  // get the list of features
  $vars = array();
  $sql  = "SELECT DISTINCT F.feature_id, F.name, F.uniquename, O.genus, O.species, CVT.name as feature_type ".
          "FROM {feature} F ".
          "  INNER JOIN {organism} O on O.organism_id = F.organism_id ".
          "  INNER JOIN {cvterm} CVT on CVT.cvterm_id = F.type_id ";
  if ($analysis_name) {
     $sql .= "  INNER JOIN {analysisfeature} AF on AF.feature_id = F.feature_id ".
             "  INNER JOIN {analysis} A on AF.analysis_id = A.analysis_id ";
  }         
  $sql .= "WHERE (1=1) ";
  if ($org_commonname) {
     $sql .= "AND O.common_name = '%s' ";
     $vars[] = $org_commonname;
  }
  if ($genus) {
     $sql .= "AND O.genus = '%s' ";
     $vars[] = $genus;
  }
  if ($species) {
     $sql .= "AND O.species = '%s' ";
     $vars[] = $species;
  }
  if ($type) {
    $sql .= "AND CVT.name = '%s' ";
    $vars[] = $type; 
  }
  if ($feature_name) {
     $sql .= "AND F.name = '%s'";
     $vars[] = $feature_name;
  }
  if ($analysis_name) {
     $sql .= "AND A.name = '%s'";
     $vars[] = $analysis_name;
  }
  $num_bases_per_line = 50;
  $q = chado_query($sql, $vars);
  while ($feature = db_fetch_object($q)) {
    $feature_id = $feature->feature_id;
    $feature_name = "$feature->uniquename $feature->name $feature->feature_type ($feature->genus $feature->species)";
      
    $sequence = tripal_feature_get_formatted_sequence($feature_id, $feature_name, 
      $num_bases_per_line, $derive_from_parent, $aggregate, $output_format,
      $upstream, $downstream, $sub_features);
    print $sequence;
  }
}