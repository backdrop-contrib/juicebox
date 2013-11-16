<?php


/**
 * @file
 * A connector plugin containing methods for structuring Drupal field data into
 * a Juicebox gallery.
 */


class JuiceboxConnectorField extends JuiceboxConnector {
  
  /**
   * Base properies specific to this connector.
   */
  public $fieldAccess;
  public $entityAccess;
  protected $entityType;
  protected $entityId;
  protected $fieldName;
  protected $displayName;
  protected $entity;
  
  /**
   * Constructor
   * 
   * @param array $xml_path_args
   *   An indexed array of XML path arguments that describe this gallery (and
   *   make up its XML URL). This information uniquely identifies the gallery
   *   and contains all the descriptive data needed to (re)build it.
   * @param mixed $data
   *   (Optional) Loaded Drupal data that the gallery can be built from. When
   *   the embed code is being generated for a Drupal display formatter this
   *   data will typically be available already. However when building the XML
   *   this will be NULL, and we will need to be load it ourselves (based on the
   *   XML args) later.
   * @param object $gallery
   *   An empty gallery object, implementing JuiceboxGalleryInterface, that can
   *   be built into a Juicebox gallery.
   * @param array $library
   *   An associative array of the juicebox library information as structured by
   *   the libraries API.
   */
  function __construct($xml_path_args, $data = NULL, JuiceboxGalleryInterface $gallery, $library) {
    // Run common construct logic.
    parent::__construct($xml_path_args, $data, $gallery, $library);
    // We need 5 data sources to build a file-based gallery (the type along
    // with 4 identifiers for the field data).
    if (empty($this->xmlPathArgs) || count($this->xmlPathArgs) < 5) {
      throw new Exception(t('Cannot instantiate field-based Juicebox gallery due to insufficient XML path args.'));
    }
    // Set data sources as properties.
    $this->entityType = $xml_path_args[1];
    $this->entityId = $xml_path_args[2];
    $this->fieldName = $xml_path_args[3];
    $this->displayName = $xml_path_args[4];
    // Grab the loaded entity as well (typically from static cache).
    $entities = entity_load($this->entityType, array($this->entityId));
    $this->entity = reset($entities);
  }
   
  /**
   * Load the Drupal data needed to build the gallery.
   */
  function load() {
    $entity = $this->entity;
    // Get the bundle details.
    $info = entity_get_info($this->entityType);
    if (empty($info['entity keys']['bundle'])) {
      $bundle = $this->entityType;
    }
    else {
      $bundle = $entity->{$info['entity keys']['bundle']};
    }
    // Get the instance and display details.
    $instance = field_info_instance($this->entityType, $this->fieldName, $bundle);
    if (!empty($instance['display'])) {
      // Get the settings.
      $this->data['settings'] = $instance['display']['default']['settings'];
      if (isset($instance['display'][$this->displayName]['settings'])) {
        $this->data['settings'] = $instance['display'][$this->displayName]['settings'];
      }
      // Build the field. This should easily trigger all field hooks.
      $built_field = field_view_field($this->entityType, $entity, $this->fieldName, $instance['display']);
      if ($built_field['#items']) {
        $this->data['items'] = $built_field['#items'];
        // We have the field level access details already, so set then for
        // further reference.
        $this->fieldAccess = $built_field['#access'];
        return;
      }
    }
    // If we got here there was a problem loading the data.
    throw new Exception(t('There was an exception while loading Drupal data for a field-based Juicebox gallery.'));
  }
  
  /**
   * Check access to the Drupal data that will be used to build the gallery.
   * 
   * @return boolean
   *   Return TRUE if access is allowed for the current user and FALSE if not.
   *   Can also return NULL if access cannot be determined.
   */
  function access() {
    $access = TRUE;
    // Check field-level access.
    if (!isset($this->fieldAccess)) {
      $field = field_info_field($this->fieldName);
      $this->fieldAccess = field_access('view', $field, $this->entityType, $this->entity);
    }
    $access = ($access && $this->fieldAccess);
    // Check entity-level access. 
    if (!isset($this->entityAccess)) {
      // If the Entity API module is installed we can use entity_access() to
      // check access for numerous entity types via their access callbacks.
      // All core entities, and many custom ones, can be handled here.
      if (module_exists('entity')) {
        $this->entityAccess = entity_access('view', $this->entityType, $this->entity);
      }
      // If we can't do a check with entity_access() we only maintain checks
      // for popular core entity types that provide thier own explicit access
      // functions.
      else {
        switch ($this->entityType) {
          case 'node':
            $this->entityAccess = node_access('view', $this->entity);
            break;
          case 'user':
            $this->entityAccess = user_view_access($this->entity);
            break;
          default:
            // Log a warning and return NULL if we can't do a conclusive
            // check.
            watchdog('juicebox', 'Could not verify view access for entity type %type while building Juicebox data. This may have resulted in a broken gallery display. You may be able to remove this error by installing the Entity API module and ensuring that an access callback exists for entities of type %type.', array('%type' => $entity_type), WATCHDOG_ERROR);
            return;
        }
      }
    }
    $access = ($access && $this->entityAccess);
    return $access;
  }
  
  /**
   * Build the gallery based on loaded Drupal data specific to this connector.
   */
  protected function buildGallery() {
    $settings = $this->data['settings'];
    $images = array();
    foreach ($this->data['items'] as $id => $item) {
      // Initiate source details.
      $image_src = $thumb_src = $unstyled_src = '';
      $src = file_create_url($item['uri']);
      if ($src) {
        $image_src = $thumb_src = $unstyled_src = $src;
      }
      // Chech to see if we are dealing with an incompatible file.
      $incompatible_file_alt = _juicebox_incompatible_file_alternative($item, $this->library);
      if ($incompatible_file_alt && !empty($settings['incompatible_file_action'])) {
        if ($settings['incompatible_file_action'] == 'skip') {
          continue;
        }
        else {
          $image_src = $thumb_src = $incompatible_file_alt;
        }
      }
      // If we have a compatible file, try to style it.
      else {
        if (!empty($settings['image_style'])) {
          $image_src = image_style_url($settings['image_style'], $item['uri']);
        }
        if (!empty($settings['thumb_style'])) {
          $thumb_src = image_style_url($settings['thumb_style'], $item['uri']);
        }
      }
      // Get the image title.
      $title = '';
      // If we have an incompatible file and are configured to show a link, set
      // the title text as the link.
      if ($incompatible_file_alt && $settings['incompatible_file_action'] == 'show_icon_and_link') {
        $anchor = !empty($item['description']) ? $item['description'] : $item['filename'];
        $title = l(check_plain($anchor), $unstyled_src);
      }
      elseif (!empty($item[$settings['title_source']])) {
        // If a field option is specified we need to try to get the value from a
        // file entity.
        if (strpos($settings['title_source'], 'field_') === 0) {
          $title = _juicebox_get_field_value_from_file_entity($item['fid'], $settings['title_source']);
        }
        else {
          $title = check_markup($item[$settings['title_source']]);
        }
      }
      // Get the image caption.
      $caption = '';
      if (!empty($item[$settings['caption_source']])) {
        // If a field option is specified we need to try to get the value from a
        // file entity.
        if (strpos($settings['caption_source'], 'field_') === 0) {
          $caption = _juicebox_get_field_value_from_file_entity($item['fid'], $settings['caption_source']);
        }
        else {
          $caption = check_markup($item[$settings['caption_source']]);
        }
      }
      // Get the linkURL.
      $image_link_src = $unstyled_src;
      if (!empty($settings['linkurl_source']) && !$incompatible_file_alt) {
        $image_link_src = $image_src;
      }
      // Add each image to the gallery.
      $images_src_data = array(
        'image_url' => $image_src,
        'thumb_url' => $thumb_src,
        'link_url' => $image_link_src,
        'link_target' => $settings['linkurl_target'],
      );
      $filter = !empty($settings['apply_markup_filter']);
      $this->gallery->addImage($images_src_data, $title, $caption, $filter);
    }   
    $this->setOptions($settings);
  }  

}
