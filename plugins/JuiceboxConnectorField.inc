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
   * Build the gallery based on loaded field data.
   */
  protected function buildGallery() {
    $settings = $this->data['settings'];
    // Attach the file entities to the items for reference.
    $this->appendFilesToItems($this->data['items']);
    foreach ($this->data['items'] as $id => $item) {
      // Calculate the source data that Juicebox requires.
      $src_data = $this->getImageSrcData($item, $settings['image_style'], $item, $settings['thumb_style'], $settings);
      // Short-circut this iteration if skipping an incompatible file.
      if (!$src_data['juicebox_compatible'] && $settings['incompatible_file_action'] == 'skip') {
        continue;
      }
      // Set the image title. If we have an incompatible file and are configured
      // to show a link, set the title text as the link.
      if (!$src_data['juicebox_compatible'] && $settings['incompatible_file_action'] == 'show_icon_and_link') {
        $anchor = !empty($item['description']) ? $item['description'] : $item['filename'];
        $title = l($anchor, $src_data['link_url']);
      }
      else {
        $title = $this->getTextFromItem($item, $settings['title_source']);
      }
      // Set the image caption.
      $caption = $this->getTextFromItem($item, $settings['caption_source']);
      // Add this image to the gallery.
      $this->gallery->addImage($src_data, $title, $caption, variable_get('juicebox_apply_markup_filter', TRUE));
    }
    // Add all gallery options directly from the field settings.
    $this->setOptions($settings);
  }

  /**
   * Utility to get sanitized text directly from a file item.
   * 
   * This method will attempt to extract text, in a format safe for display,
   * from the data contained within a file item.
   *
   * @param array $item
   *   An associative array of file information.
   * @param string $source
   *   The key within the $item array that contains the text that we want to
   *   extract.
   * @return string
   *   Safe text for output or an empty string if no text can be extracted.
   * 
   * @see JuiceboxConnectorField::confGetTextSources()
   */
  protected function getTextFromItem($item, $source) {
    $text = '';
    if (!empty($item[$source])) {
      // If a field option is specified we need to try to get the value from a
      // file entity.
      if (strpos($source, 'field_') === 0) {
        // See if we can already reference the file entity from the item itself.
        // If not we have to load it (most likely from the static cache).
        if (!empty($item['file_entity'])) {
          $file_entity = $item['file_entity'];
        }
        else {
          $file_entity = file_load($item['fid']);
        }
        $field = field_get_items('file', $file_entity, $source);
        if (!empty($field[0]['safe_value'])) {
          $text = $field[0]['safe_value'];
        }
      }
      else {
        $text = check_markup($item[$source]);
      }
    }
    return $text;
  }
  
  /**
   * Utility to fetch the title and caption source options for field-based
   * galleries (primarily for File Entity and Media module support).
   * 
   * Note that this may be called when building Drupal configuration forms that
   * are managed outside of this object.
   *
   * @param array $instance
   *   An associative array containing the Drupal field instance information for
   *   a Juicebox field.
   * @return array
   *   An associative array representing the key => label pairs for each title
   *   and caption source option. Each key will match a keyed value on the file
   *   item source array when the gallery is built.
   * 
   * @see JuiceboxConnectorField::getTextFromItem()
   */
  static function confGetTextSources($instance) {
    // If this is a standard image field, using the image widget, we can use
    // core image "alt" and "title" values if they are active.
    if (!empty($instance['widget']['module']) && $instance['widget']['module'] == 'image') {
      if (!empty($instance['settings']['alt_field'])) {
        $text_source_options['alt'] = t('Image - Alt text (processed by fallback text format)');
      }
      if (!empty($instance['settings']['title_field'])) {
        $text_source_options['title'] = t('Image - Title text (processed by fallback text format)');
      }
    }
    // If file entity is installed we can use fields attached to the "image"
    // file entity.
    if (module_exists('file_entity')) {
      // Get the fields that are available on the image file type.
      $image_fields = field_info_instances('file', 'image');
      foreach ($image_fields as $image_field_name => $image_field) {
        // Only text-based fields should be options.
        if (!empty($image_field['widget']['module']) && $image_field['widget']['module'] == 'text') {
          $text_source_options[$image_field_name] = t('File Entity Image - @label', array('@label' => $image_field['label']));
        }
      }
    }
    return $text_source_options;
  }

}
