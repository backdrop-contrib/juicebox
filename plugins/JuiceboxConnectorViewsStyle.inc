<?php


/**
 * @file
 * A connector plugin containing methods for structuring a Drupal views display
 * into a Juicebox gallery.
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
    }
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
    // Get rendered field data that's not available in $view->result
    $rendered_fields = $view->style_plugin->render_fields($view->result);
    $images = array();
    foreach ($view->result as $row_index => $row) {
      // First make sure that we have image field data to work with.
      // This prevents php errors from coming up if the user has not yet
      // configured their view page display or if the view lists items that
      // don't contain an image.
      $field_image_name = 'field_' . $settings['image_field'];
      $field_thumb_name = 'field_' . $settings['thumb_field'];
      if (!empty($row->{$field_image_name}[0]['raw']['uri']) && !empty($row->{$field_thumb_name}[0]['raw']['uri'])) {
        $field_image_uri = $row->{$field_image_name}[0]['raw']['uri'];
        $unstyled_image_src = file_create_url($field_image_uri);
        // Get the main image source.
        $image_src = $unstyled_image_src;
        if (!empty($settings['image_field_style'])) {
          $image_src = image_style_url($settings['image_field_style'], $field_image_uri);
        }
        // Get the thumbnail source.
        $thumb_src = $unstyled_image_src;
        $field_thumb_uri = $row->{$field_thumb_name}[0]['raw']['uri'];
        if (!empty($settings['thumb_field_style'])) {
          $thumb_src = image_style_url($settings['thumb_field_style'], $field_thumb_uri);
        }
        // Get the image title.
        $title = '';
        if (!empty($settings['title_field']) && !empty($rendered_fields[$row_index][$settings['title_field']])) {
          $title = $rendered_fields[$row_index][$settings['title_field']];
        }
        // Get the image caption.
        $caption = '';
        if (!empty($settings['caption_field']) && !empty($rendered_fields[$row_index][$settings['caption_field']])) {
          $caption = $rendered_fields[$row_index][$settings['caption_field']];
        }
        // Get the linkURL.
        $image_link_src = $unstyled_image_src;
        if (!empty($settings['linkurl_source'])) {
          if ($settings['linkurl_source'] == 'image_styled') {
            $image_link_src = $image_src;
          }
          elseif (!empty($rendered_fields[$row_index][$settings['linkurl_source']])) {
            $image_link_src = $rendered_fields[$row_index][$settings['linkurl_source']];
          }
        }
        // Add each image to the gallery.
        $images_src_data = array(
          'image_url' => $image_src,
          'thumb_url' => $thumb_src,
          'link_url' => $image_link_src,
          'link_target' => $settings['linkurl_target'],
        );
        $filter = variable_get('juicebox_apply_markup_filter', TRUE);
        $this->gallery->addImage($images_src_data, $title, $caption, $filter);
      }
    }
    if ($settings['show_title']) {
      $this->gallery->addOption('gallerytitle', check_plain($view->get_title()));
    }
    $this->setOptions($settings);
  }

}