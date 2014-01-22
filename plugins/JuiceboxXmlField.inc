<?php


/**
 * @file
 * Juicebox XML loader that's used to load (and build via loaded methods) the
 * XML associated with a Drupal field formatter plugin.
 */


/**
 * Class to load and build the XML associated with a Drupal field formatter
 * plugin.
 */
class JuiceboxXmlField implements JuiceboxXmlInterface {

  // Injected gallery object to build.
  protected $juicebox;
  // Base properties that reference source data.
  protected $idArgs;
  protected $entityType;
  protected $entityId;
  protected $fieldName;
  protected $displayName;
  protected $entity;
  // Helper properties for access checks.
  protected $fieldAccess;
  protected $entityAccess;
  // Dynamic loaded data storage.
  protected $settings = array();
  protected $items = array();

  /**
   * Constructor.
   *
   * @param array $id_args
   *   An indexed array of arguments that describe this gallery (and make up its
   *   XML URL). This information uniquely identifies the gallery and contains
   *   all the descriptive data needed to load it.
   */
  public function __construct($id_args) {
    // We need 5 data sources to build a file-based gallery (the type along
    // with 4 identifiers for the field data).
    if (empty($id_args) || count($id_args) < 5) {
      throw new Exception(t('Cannot initiate field-based Juicebox XML due to insufficient ID args.'));
    }
    // Set data sources as properties.
    $this->idArgs = $id_args;
    $this->entityType = $id_args[1];
    $this->entityId = $id_args[2];
    $this->fieldName = $id_args[3];
    $this->displayName = $id_args[4];
    // Grab the loaded entity as well (typically from static cache).
    $entities = entity_load($this->entityType, array($this->entityId));
    $this->entity = reset($entities);
  }

  /**
   * {@inheritdoc}
   */
  public function access() {
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
   * Load the Drupal data needed to build the gallery.
   *
   * This produces the same data that's made available to the formatter plugin
   * when rendering the HTML.
   */
  protected function loadFormatterData() {
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
      $settings = $instance['display']['default']['settings'];
      if (isset($instance['display'][$this->displayName]['settings'])) {
        $settings = $instance['display'][$this->displayName]['settings'];
      }
      $this->settings = $settings;
      // Build the field. This should easily trigger all field hooks.
      $built_field = field_view_field($this->entityType, $entity, $this->fieldName, $instance['display']);
      if ($built_field['#items']) {
        $this->items = $built_field['#items'];
        // We have the field level access details already, so set them for
        // further reference.
        $this->fieldAccess = $built_field['#access'];
        // We now have the same data available that the formatter plugin would
        // have.
        return;
      }
    }
    // If we got here there was a problem loading the data.
    throw new Exception(t('There was an exception while loading Drupal data for a field-based Juicebox gallery.'));
  }

  /**
   * {@inheritdoc}
   */
  public function getXml() {
    // (Lazy) Load the data needed to build the gallery.
    $this->loadFormatterData();
    // Initalize the gallery.
    $juicebox = juicebox();
    $juicebox->init($this->idArgs, $this->settings, $this->items);
    // Build the gallery via the field formatter methods. In D7 the field
    // formatter is NOT a plugin, so we have to make a global function call for
    // this. This will change in D8.
    juicebox_field_build_gallery($juicebox, $this->items);
    // Render the XML.
    return $juicebox->renderXml();
  }

}
