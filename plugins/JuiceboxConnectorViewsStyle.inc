<?php


/**
 * @file
 * A connector plugin containing methods for structuring a Drupal views display
 * into a Juicebox gallery.
 */


/**
 * Class to structure Drupal view data into a Juicebox gallery.
 */
class JuiceboxConnectorViewsStyle extends JuiceboxConnector {

  /**
   * Base properies specific to this connector.
   */
  protected $viewName;
  protected $viewDisplay;
  protected $viewArgs;

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
  function __construct($xml_path_args, view $data = NULL, JuiceboxGalleryInterface $gallery, $library) {
    // Run common construct logic.
    parent::__construct($xml_path_args, $data, $gallery, $library);
    // We need at least 3 data sources to build a view-based gallery (the type
    // along with 2 identifiers for the view display).
    if (empty($this->xmlPathArgs) || count($this->xmlPathArgs) < 3) {
      throw new Exception(t('Cannot instantiate field-based Juicebox gallery due to insufficient XML path args.'));
    }
    // Set data sources as properties.
    $this->viewName = $xml_path_args[1];
    $this->viewDisplay = $xml_path_args[2];
    $this->viewArgs = array_slice($xml_path_args, 3);
  }

  /**
   * Load the Drupal data needed to build the gallery.
   */
  function load() {
    // Load the view.
    $view = views_get_view($this->viewName);
    if ($view) {
      // Execute the view. This is easiest to do by simply calling the preview
      // method (as it handles all needed sub-processes).
      $view->preview($this->viewDisplay, $this->viewArgs);
      $this->data = $view;
      return;
    }
    // If we got here there was a problem loading the data.
    throw new Exception(t('There was an exception while loading Drupal data for a view-based Juicebox gallery.'));
  }

  /**
   * Check access to the Drupal data that will be used to build the gallery.
   *
   * @return boolean
   *   Return TRUE if access is allowed for the current user and FALSE if not.
   *   Can also return NULL if access cannot be determined.
   */
  function access() {
    if (!$this->data) {
      $view = views_get_view($this->viewName);
      return $view->access($this->viewDisplay);
    }
    return $this->data->access($this->viewDisplay);
  }

  /**
   * Build the gallery based on loaded Drupal data specific to this connector.
   */
  protected function buildGallery() {
    $view = $this->data;
    $settings = $view->style_plugin->options;
    $rendered_fields = $view->style_plugin->render_fields($view->result);
    // Get all row image data in the format of Drupal file field items.
    $image_items = $thumb_items = $this->getItems($settings['image_field']);
    if ($settings['image_field'] != $settings['thumb_field']) {
      $thumb_items = $this->getItems($settings['thumb_field']);
    }
    // Iterate through each view row and calculate the gallery-specific details.
    foreach ($image_items as $row_index => $image_item) {
      // Make sure each main image has a thumb item.
      $thumb_item = !empty($thumb_items[$row_index]) ? $thumb_items[$row_index] : $image_item;
      // Calculate the source data that Juicebox requires.
      $src_data = $this->getImageSrcData($image_item, $settings['image_field_style'], $thumb_item, $settings['thumb_field_style'], $settings);
      // Short-circut this iteration if skipping an incompatible file.
      if (!$src_data['juicebox_compatible'] && $settings['incompatible_file_action'] == 'skip') {
        continue;
      }
      // Check if the linkURL should be customized based on view settings.
      if (!empty($settings['linkurl_source']) && !empty($rendered_fields[$row_index][$settings['linkurl_source']])) {
        $src_data['link_url'] = $rendered_fields[$row_index][$settings['linkurl_source']];
      }
      // Set the image title.
      $title = '';
      // If we have an incompatible file the title may need special handeling.
      if (!$src_data['juicebox_compatible'] && $settings['incompatible_file_action'] == 'show_icon_and_link') {
        $anchor = !empty($image_item['description']) ? $image_item['description'] : $image_item['filename'];
        $title = l($anchor, $src_data['link_url']);
      }
      elseif (!empty($settings['title_field']) && !empty($rendered_fields[$row_index][$settings['title_field']])) {
        $title = $rendered_fields[$row_index][$settings['title_field']];
      }
      // Set the image caption.
      $caption = '';
      if (!empty($settings['caption_field']) && !empty($rendered_fields[$row_index][$settings['caption_field']])) {
        $caption = $rendered_fields[$row_index][$settings['caption_field']];
      }
      // Add this image to the gallery.
      $this->gallery->addImage($src_data, $title, $caption, variable_get('juicebox_apply_markup_filter', TRUE));
    }
    // Add all gallery options directly from the display settings.
    $this->setOptions($settings);
    if ($settings['show_title']) {
      $this->gallery->addOption('gallerytitle', check_plain($view->get_title()), FALSE);
    }
  }

  /**
   * Utility to get the item arrays that contain image data from view rows.
   *
   * The items that this method returns are array representations of Drupal file
   * objects. This type of array is often used by Drupal to represent file
   * fields, so this a common format to work with.
   *
   * @param string $source_field
   *   The view field source that will contain a file identifer. The exact part
   *   of the row data to get the file identifer from will depend on the field
   *   type, and this method will resolve that based on the view's field handler
   *   details.
   * @return array
   *   An indexed array, keyed by row id, of file field items that were
   *   extracted based on row data.
   *
   * @see JuiceboxConnectorViewStyle::confGetFieldSources()
   */
  protected function getItems($source_field) {
    $view = $this->data;
    // Get the field source options and make sure the passed-source is valid.
    $source_options = self::confGetFieldSources($view);
    if (empty($source_options['field_options_images_type'][$source_field])) {
      throw new Exception(t('Empty or invalid field source @source detected for Juicebox view-based gallery.', array('@source' => $source_field)));
    }
    else {
      $source_type = $source_options['field_options_images_type'][$source_field];
    }
    $fids = array();
    $items = array();
    // Pass 1 - get the fids based on the source type.
    foreach ($view->result as $row_index => $row) {
      switch ($source_type) {
        case 'file_base':
          // This is a file-based view so the fid is already in the row data.
          if (!empty($row->fid)) {
            $fids[$row_index] = $row->fid;
          }
          continue;
        case 'file_id_field':
          // The source is a file ID field so we can fetch the fid from row
          // data.
          if (!empty($row->{$view->field[$source_field]->field_alias})) {
            $fids[$row_index] = $row->{$view->field[$source_field]->field_alias};
          }
          continue;
        case 'file_field':
          // The source is an image or file field so views should have already
          // loaded the related file entities which will include the fid. In
          // this case we also already have our file field item array.
          if (!empty($row->{'field_' . $source_field}[0]['raw'])) {
            $fids[$row_index] = $row->{'field_' . $source_field}[0]['raw']['fid'];
            $items[$row_index] = $row->{'field_' . $source_field}[0]['raw'];
          }
      }
    }
    if (empty($items)) {
      // Bulk load all file entities.
      $file_entities = file_load_multiple($fids);
      // Pass 2 - Get field items from the file entities.
      foreach ($fids as $row_index => $fid) {
        $items[$row_index] = (array) $file_entities[$fid];
      }
    }
    return $items;
  }

  /**
   * Utility to determine which view fields can be used for image data.
   *
   * This method will extract a list of fields that can be used as "sources"
   * for a Juicebox gallery along with other useful field information. Note that
   * this may be called when building Drupal configuration forms that are
   * managed outside of this object.
   *
   * @param array $view
   *   A view object that contains all configuration data (it probably does not
   *   need to be built/executed).
   * @return array
   *   An associative array containing a breakdown of field data that can be
   *   referenced by other build methods, including:
   *   - field_options_image: An associative array, keyed by field id, of fields
   *     that can be used as Juicebox gallery image sources.
   *   - field_options_image_type: An associative array, keyed by field id, of
   *     field "types" for all fields listed in 'field_options_image' above.
   *   - field_options: An associative array, keyed by field id, of fields that
   *     cannot be used as Juicebox gallery image sources, but may be useful
   *     for other purposes (text and caption sorces, etc.)
   *
   * @see JuiceboxConnectorViewStyle::getFileItems()
   */
  static function confGetFieldSources($view) {
    $options = array(
      'field_options_images' => array(),
      'field_options_images_type' => array(),
      'field_options' => array(),
    );
    $field_handlers = $view->display_handler->display->handler->get_handlers('field');
    // If we have a "file" based view we can use the base ids from the view as
    // fids, so add a "base" option to the image source list.
    if (!empty($view->base_table) && $view->base_table == 'file_managed') {
      $options['field_options_images']['file_base'] = t('Base file ID');
      $options['field_options_images_type']['file_base'] = 'file_base';
    }
    // Get the label for each field.
    foreach ($field_handlers as $field => $handler) {
      $label = $handler->label();
      $name = $label ? $label : $handler->ui_name();
      // Separate image fields from non-image fields. For image fields we can
      // work with fids and fields of type image or file.
      if (isset($handler->field) && $handler->field == 'fid') {
        $options['field_options_images'][$field] = $name;
        $options['field_options_images_type'][$field] = 'file_id_field';
      }
      elseif (isset($handler->field_info['type']) && ($handler->field_info['type'] == 'image' || $handler->field_info['type'] == 'file')) {
        $options['field_options_images'][$field] = $name;
        $options['field_options_images_type'][$field] = 'file_field';
      }
      else {
        $options['field_options'][$field] = $name;
      }
    }
    return $options;
  }

}