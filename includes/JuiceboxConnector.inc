<?php


/**
 * @file
 * This is a base connector containing the building blocks for structuring
 * Drupal data into a Juicebox gallery. This can be extended into specifc
 * plugins to handle different Drupal constructs (fields, views, etc.).
 */


abstract class JuiceboxConnector {
  
  /**
   * Base properies of structured data to manage the loading/handling of
   * Drupal info that will be processed into a gallery.
   */
  public $xmlPathArgs = array();
  public $type;
  // Library API information is stored for reference.
  public $library;
  // The raw Drupal data that will be built into a gallery is stored in mixed
  // format depending on the connector type.
  protected $data;
  // The gallery object is injected and serves as a direct interface to
  // Juicebox once Drupal data is loaded.
  public $gallery;
  // We maintain a static cache of "built" $gallery data objects, keyed by
  // gallery ID.
  static protected $builtGalleries = array();
  
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
  function __construct(&$xml_path_args, $data = NULL, JuiceboxGalleryInterface $gallery, $library) {
    // Set data if we have it. If not we will need to get it later with the
    // load() method.
    if ($data) {
      $this->data = $data;
    }
    // Make sure that we have additional args to describe the Drupal data.
    if (empty($xml_path_args)) {
      throw new Exception(t('Cannot instantiate Juicebox gallery due to insufficient XML path args.'));
    }
    // Store the empty gallery object and some reference info.
    $this->gallery = $gallery;
    $this->library = $library;
    $this->type = $xml_path_args[0];
    $this->xmlPathArgs = $xml_path_args;
  }
  
  /**
   * Load the Drupal data needed to build the gallery.
   * 
   * This method should fully populate the $data propery.
   */
  abstract function load();
  
  /**
   * Check access to the Drupal data that will be used to build the gallery.
   * 
   * @return boolean
   *   Return TRUE if access is allowed for the current user and FALSE if not.
   *   Can also return NULL if access cannot be determined.
   */
  abstract function access();
  
  /**
   * Build the gallery based on loaded Drupal data specific to this connector.
   * 
   * This method should fully populate the $gallery object propery.
   */
  abstract protected function buildGallery();
  
  /**
   * Build the gallery.
   * 
   * This is essentially a wrapper for buildGallery() that ensures some common
   * pre/post-build tasks are done.
   * 
   * @param boolean $reset
   *   Whether-or-not to bypass, and reset, any statically cached gallery data.
   */
  function build($reset = FALSE) {
    // We maintain a static cache of built gallery data for efficiency. For
    // example, there may be cases where the load() method triggers a series
    // of formatter functions that also build the gallery data. Becuase build()
    // is often called right after load() (e.g. during an XML request) this can
    // prevent the build logic from running redundantly.
    if (!empty(self::$builtGalleries[$this->gallery->id]) && !$reset) {
      $this->gallery = self::$builtGalleries[$this->gallery->id];
    }
    else {
      // Build the gallery based on specific plugin logic.
      $this->buildGallery();
      self::$builtGalleries[$this->gallery->id] = $this->gallery;
    }
    // Allow other modules to alter the built gallery data before it's rendered.
    drupal_alter('juicebox_gallery', $this);
  }
  
  /**
   * Helper to extract Juicebox gallery options from common Drupal display
   * settings, and add them to the gallery.
   *
   * Some common Juicebox configuration options are set via a GUI and others
   * are set as manual strings. This method fetches all of these values from
   * drupal settings data and merges them into the gallery. Note that this only
   * accounts for common settings.
   * 
   * @param array $settings
   *   An associative array of common Drupal settings for a gallery.
   */
  function setOptions($settings) {
    // Get the string options set via the GUI.
    foreach (array('jlib_galleryWidth', 'jlib_galleryHeight', 'jlib_backgroundColor', 'jlib_textColor', 'jlib_thumbFrameColor') as $name) {
      if (isset($settings[$name])) {
        $name_real = str_replace('jlib_', '', $name);
        $this->gallery->addOption(drupal_strtolower($name_real), trim(check_plain($settings[$name])));
      }
    }
    // Get the bool options set via the GUI.
    foreach (array('jlib_showOpenButton', 'jlib_showExpandButton', 'jlib_showThumbsButton', 'jlib_useThumbDots', 'jlib_useFullscreenExpand') as $name) {
      if (isset($settings[$name])) {
        $name_real = str_replace('jlib_', '', $name);
        $this->gallery->addOption(drupal_strtolower($name_real), (!empty($settings[$name]) ? 'TRUE' : 'FALSE'));
      }
    }
    // Merge-in the manually assigned options making sure they take priority
    // over any conflicting GUI options.
    $manual_options = explode("\n", $settings['manual_config']);
    foreach ($manual_options as $option) {
      $option = trim($option);
      if (!empty($option)) {
        // Each manual option has only been validated (on input) to be in the
        // form optionName="optionValue". Now we need split and sanitize the
        // values.
        $matches = array();
        preg_match('/^([A-Za-z0-9]+?)="([^"]+?)"$/u', $option, $matches);
        list($full_match, $name, $value) = $matches;
        $name = drupal_strtolower($name);
        $this->gallery->addOption(drupal_strtolower($name), check_plain($value));
      }
    }
  }
  
  /**
   * Render the embed code for a Juicebox gallery.
   *
   * This is a replacement for the gallery object's own getEmbed() method. It
   * uses the Drupal theme system and allows for more flexible and comprehensive
   * embed code output.
   * 
   * @param array $settings
   *   An associative array of common Drupal settings for a gallery.
   */
  function renderEmbed($settings) {
    return theme('juicebox_embed_markup', array('gallery' => $this->gallery, 'settings' => $settings));
  }
  
  /**
   * Get the "base" values of common Drupal settings used to describe a gallery.
   * This is used for the management of default configuration values.
   *
   * @return array
   *   An associative array of base/default configuration values. 
   */
  static function confBaseOptions() {
    return array(
      'jlib_galleryWidth' => '100%',
      'jlib_galleryHeight' => '100%',
      'jlib_backgroundColor' => '#222222',
      'jlib_textColor' => 'rgba(255,255,255,1)',
      'jlib_thumbFrameColor' => 'rgba(255,255,255,.5)',
      'jlib_showOpenButton' => 1,
      'jlib_showExpandButton' => 1,
      'jlib_showThumbsButton' => 1,
      'jlib_useThumbDots' => 0,
      'jlib_useFullscreenExpand' => 0,
      'manual_config' => '',
      'custom_parent_classes' => '',
      'apply_markup_filter' => 1,
      'linkurl_source' => '',
      'linkurl_target' => '_blank',
      'incompatible_file_action' => 'show_icon_and_link',
    );
  }
  
  /**
   * Get common elements for Juicebox configuration forms.
   *
   * Several Juicebox gallery types can share common options and structures.
   * These are merged into the appropriate forms via a call to this method.
   *
   * @param array $form
   *   The Drupal form array that common elements should be added to.
   * @param array $settings
   *   An associative array containing all the current settings for a Juicebox
   *   gallery (used to set default values).
   * @return array
   *   The common form elements merged within a form array.
   */
  static function confBaseForm($form, $settings) {
    // Get locally installed library details.
    $library = juicebox_detect_library();
    $disallowed_conf = array();
    if (!empty($library)) {
      // If we don't have a known version of the Juicebox library, just show a
      // generic warning.
      if (empty($library['version'])) {
        $notification_top = t('<strong>Notice:</strong> Your Juicebox Library version could not be detected. Some options below may not function correctly.');
      }
      // If this version does not support some LITE optins, show a message.
      elseif (!empty($library['disallowed_conf'])) {
        $disallowed_conf = $library['disallowed_conf'];
        $notification_top = t('<strong>Notice:</strong> You are currently using Juicebox library version <strong>@version</strong> which is not compatible with some of the options listed below. These options will appear disabled until you upgrade to the most recent Juicebox library version.', array('@version' => $library['version']));
        $notification_label = t('&nbsp;(not available in @version)', array('@version' => $library['version']));
      }
    }
    // If the library itself is not installed, display formal error message.
    else {
      $notification_top = t('The Juicebox Javascript library does not appear to be installed. Please download and install the most recent version of the Juicebox library.');
      drupal_set_message($notification_top, 'error');
    }
    $form['juicebox_config'] = array(
      '#type' => 'fieldset',
      '#title' => t('Juicebox Library - Lite Config'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#description' => !empty($notification_top) ? '<p>' . $notification_top . '</p>' : '',
      '#weight' => 10,
    );
    $form['jlib_galleryWidth'] = array(
      '#jb_fieldset' => 'juicebox_config',
      '#type' => 'textfield',
      '#title' => t('Gallery Width'),
      '#description' => t('Set the gallery width in a standard numeric format (such as 100% or 300px).'),
      '#element_validate' => array('juicebox_element_validate_dimension'),
    );
    $form['jlib_galleryHeight'] = array(
      '#jb_fieldset' => 'juicebox_config',
      '#type' => 'textfield',
      '#title' => t('Gallery Height'),
      '#description' => t('Set the gallery height in a standard numeric format (such as 100% or 300px).'),
      '#element_validate' => array('juicebox_element_validate_dimension'),
    );
    $form['jlib_backgroundColor'] = array(
      '#jb_fieldset' => 'juicebox_config',
      '#type' => 'textfield',
      '#title' => t('Background Color'),
      '#description' => t('Set the gallery background color as a CSS3 color value (such as rgba(10,50,100,0.7) or #FF00FF).'),
    );
    $form['jlib_textColor'] = array(
      '#jb_fieldset' => 'juicebox_config',
      '#type' => 'textfield',
      '#title' => t('Text Color'),
      '#description' => t('Set the color of all gallery text as a CSS3 color value (such as rgba(255,255,255,1) or #FF00FF).'),
    );
    $form['jlib_thumbFrameColor'] = array(
      '#jb_fieldset' => 'juicebox_config',
      '#type' => 'textfield',
      '#title' => t('Thumbnail Frame Color'),
      '#description' => t('Set the color of the thumbnail frame as a CSS3 color value (such as rgba(255,255,255,.5) or #FF00FF).'),
    );
    $form['jlib_showOpenButton'] = array(
      '#jb_fieldset' => 'juicebox_config',
      '#type' => 'checkbox',
      '#title' => t('Show Open Image Button'),
      '#description' => t('Whether to show the "Open Image" button. This will link to the full size version of the image within a new tab to facilitate downloading.'),
    );
    $form['jlib_showExpandButton'] = array(
      '#jb_fieldset' => 'juicebox_config',
      '#type' => 'checkbox',
      '#title' => t('Show Expand Button'),
      '#description' => t('Whether to show the "Expand" button. Clicking this button expands the gallery to fill the browser window.'),
    );
    $form['jlib_useFullscreenExpand'] = array(
      '#jb_fieldset' => 'juicebox_config',
      '#type' => 'checkbox',
      '#title' => t('Use Fullscreen Expand'),
      '#description' => t('Whether to trigger fullscreen mode when clicking the expand button (for supported browsers).'),
    );
    $form['jlib_showThumbsButton'] = array(
      '#jb_fieldset' => 'juicebox_config',
      '#type' => 'checkbox',
      '#title' => t('Show Thumbs Button'),
      '#description' => t('Whether to show the "Toggle Thumbnails" button.'),
    );
    $form['jlib_useThumbDots'] = array(
      '#jb_fieldset' => 'juicebox_config',
      '#type' => 'checkbox',
      '#title' => t('Show Thumbs Dots'),
      '#description' => t('Whether to replace the thumbnail images with small dots.'),
      '#default_value' => $settings['jlib_useThumbDots'],
    );
    $form['juicebox_manual_config'] = array(
      '#type' => 'fieldset',
      '#title' => t('Juicebox Library - Pro / Manual Config'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#description' => '<p>' . t('Specify any additional Juicebox library configuration options (such as "Pro" options) here.<br/>Options set here always take precedence over those set in the "Lite" options above if there is a conflict.') . '</p>',
      '#weight' => 20,
    );
    $form['manual_config'] = array(
      '#jb_fieldset' => 'juicebox_manual_config',
      '#type' => 'textarea',
      '#title' => t('Pro / Manual Configuraton Options'),
      '#description' => t('Add one option per line in the format <strong>optionName="optionValue"</strong><br/>See also: http://www.juicebox.net/support/config_options'),
      '#element_validate' => array('juicebox_element_validate_config'),
    );
    $form['advanced'] = array(
      '#type' => 'fieldset',
      '#title' => t('Juicebox - Advanced Options'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#weight' => 30,
    );
    $form['apply_markup_filter'] = array(
      '#jb_fieldset' => 'advanced',
      '#type' => 'checkbox',
      '#title' => t('Filter title and caption output for compatibility with Juicebox javascript (recommended)'),
      '#description' => t('This option helps ensure title/caption output is syntactically compatible with the Juicebox javascript library by removing block-level tags.'),
    );
    $form['incompatible_file_action'] = array(
      '#jb_fieldset' => 'advanced',
      '#type' => 'select',
      '#title' => t('Incompatible File Type Handling'),
      '#options' => array('skip' => 'Bypass incompatible files', 'show_icon' => 'Show mimetype icon placehoder', 'show_icon_and_link' => 'Show mimetype icon placholder and link to file'),
      '#empty_option' => t('Do nothing'),
      '#description' => t('Specify any special handling that should be applied to files that Juicebox cannot display (non-images).'),
    );
    $form['linkurl_source'] = array(
      '#jb_fieldset' => 'advanced',
      '#type' => 'select',
      '#title' => t("LinkURL Source"),
      '#description' => t('The linkURL is an image-specific path for accessing each image outside the gallery. This is used by features such as the "Open Image Button".'),
      '#options' => array('image_styled' => 'Main Image - Styled (use this gallery\'s main image style setting)'),
      '#empty_option' => t('Main Image - Unstyled (original image)'),
    );
    $form['linkurl_target'] = array(
      '#jb_fieldset' => 'advanced',
      '#type' => 'select',
      '#title' => t('LinkURL Target'),
      '#options' => array('_blank' => '_blank', '_self' => '_self', '_parent' => '_parent', '_top' => '_top'),
      '#description' => t('Specify a target for any links that make user of the image linkURL.'),
    );
    $form['custom_parent_classes'] = array(
      '#jb_fieldset' => 'advanced',
      '#type' => 'textfield',
      '#title' => t('Custom Classes for Parent Container'),
      '#description' => t('Define any custom classes that should be added to the parent container within the Juicebox embed markup.<br/>This can be handy if you want to apply more advanced styling or dimensioning rules to this gallery via CSS. Enter as space-separated values.'),
    );
    // Set values that are directly related to each key.
    foreach($form as $conf_key => &$conf_value) {
      if (!empty($conf_value['#type']) && $conf_value['#type'] != 'fieldset') {
        $conf_value['#default_value'] = $settings[$conf_key];
        if (in_array($conf_key, $disallowed_conf)) {
          $conf_value['#title'] .= $notification_label;
          $conf_value['#disabled'] = TRUE;
        }
      }
    }
    // Add a pre render callback that will ensure that the items are nested
    // correctly into fieldsets just before display.
    $form['#pre_render'] = array('juicebox_form_pre_render_fieldsets');
    return $form;
  }

}
