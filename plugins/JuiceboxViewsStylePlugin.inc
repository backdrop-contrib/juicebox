<?php


/**
 * @file
 * Contains the Juicebox views style plugin.
 */


/**
 * Style plugin to render each item in a views list.
 *
 * @ingroup views_style_plugins
 */
class JuiceboxViewsStylePlugin extends views_plugin_style {

  /**
   * Define default plugin options.
   */
  function option_definition() {
    $options = parent::option_definition();
    // Get the base settings.
    $base_settings = JuiceboxConnector::confBaseOptions();
    // Structure the base settings in the "default" format that views wants.
    foreach ($base_settings as $setting => $value) {
      $base_settings_default[$setting] = array('default' => $value);
    }
    $options = array_merge($base_settings_default, array(
      'image_field' => array('default' => ''),
      'image_field_style' => array('default' => ''),
      'thumb_field' => array('default' => ''),
      'thumb_field_style' => array('default' => 'thumbnail'),
      'title_field' => array('default' => ''),
      'caption_field' => array('default' => ''),
      'show_title' => array('default' => 0),
    ));
    return $options;
  }

  /**
   * Define plugin options form.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    $settings = $this->options;
    // Get the active field options
    $field_options = array();
    $field_options_images = array();
    $field_handlers = $this->display->handler->get_handlers('field');
    foreach ($field_handlers as $field => $handler) {
      if ($label = $handler->label()) {
        $name = $label;
      }
      else {
        $name = $handler->ui_name();
      }
      // Separate image fields from non-image fields
      if (isset($handler->field_info['type'])) {
        if ($handler->field_info['type'] == 'image') {
          $field_options_images[$field] = $name;
        }
        else {
          $field_options[$field] = $name;
        }
      }
      else {
        $field_options[$field] = $name;
      }
    }
    // Get available image style presets
    $presets = image_style_options(FALSE);

    // Start with the "common" form elements.
    $form = JuiceboxConnector::confBaseForm($form, $settings);
    // Media and incompatible file handling are not yet available in views.
    unset($form['incompatible_file_action']);
    // Add the view-specific elements.
    $form['image_field'] = array(
      '#type' => 'select',
      '#title' => t('Image Field'),
      '#default_value' => $settings['image_field'],
      '#description' => t('The view\'s image field that should be used for each image in the gallery.'),
      '#options' => $field_options_images,
    );
    $form['thumb_field'] = array(
      '#type' => 'select',
      '#title' => t('Thumbnail Field'),
      '#default_value' => $settings['thumb_field'],
      '#description' => t('The view\'s image field that should be used for each thumbnail in the gallery. Typically you will choose the same value that was set in the "Main Image Field" option above.'),
      '#options' => $field_options_images,
    );
    $form['image_field_style'] = array(
      '#type' => 'select',
      '#title' => t('Image Field Style'),
      '#default_value' => $settings['image_field_style'],
      '#description' => t('The style formatter for the image. Any formatting settings configured on the field itself will be ignored and this style setting will always be used.'),
      '#options' => $presets,
      '#empty_option' => t('None (original image)'),
    );
    $form['thumb_field_style'] = array(
      '#type' => 'select',
      '#title' => t('Thumbnail Field Style'),
      '#default_value' => $settings['thumb_field_style'],
      '#description' => t('The style formatter for the thumbnail. Any formatting settings configured on the field itself will be ignored and this style setting will always be used.'),
      '#options' => $presets,
      '#empty_option' => t('None (original image)'),
    );
    $form['title_field'] = array(
      '#type' => 'select',
      '#title' => t('Title Field'),
      '#default_value' => $settings['title_field'],
      '#description' => t('The view\'s field that should be used for the title of each image in the gallery. Any formatting settings configured on the field itself will be respected.'),
      '#options' => $field_options,
      '#empty_option' => t('None'),
    );
    $form['caption_field'] = array(
      '#type' => 'select',
      '#title' => t('Caption Field'),
      '#default_value' => $settings['caption_field'],
      '#description' => t('The view\'s field that should be used for the caption of each image in the gallery. Any formatting settings configured on the field itself will be respected.'),
      '#options' => $field_options,
      '#empty_option' => t('None'),
    );
    $form['show_title'] = array(
      '#type' => 'checkbox',
      '#title' => t('Show Gallery Title'),
      '#default_value' => $settings['show_title'],
      '#description' => t('Show the view display title as the gallery title.'),
    );
    // Add view-sepcific field options for the linkURL setting.
    $linkurl_field_options = array();
    foreach ($field_options as $field_key => $field_name) {
      $linkurl_field_options[$field_key] = t('Field') . ' - ' . $field_name;
    }
    $form['linkurl_source']['#description'] = $form['linkurl_source']['#description'] . '</br><strong>' . t('If using a field source it must render a properly formatted URL and nothing else.') . '</strong>';
    $form['linkurl_source']['#options'] = array_merge($form['linkurl_source']['#options'], $linkurl_field_options);
  }

  /**
   * Define validation rules for the plugin display.
   */
  function validate() {
    $errors = parent::validate();
    // Make sure block caching is not enabled. With block caching enabled the
    // Juicebox javascript library may not get included.
    if (isset($this->display->handler->options['block_caching']) && $this->display->handler->options['block_caching'] != DRUPAL_NO_CACHE) {
      $errors[] = t('The Juicebox style cannot be used with block caching. Please disable the "Block caching" option for this display.');
    }
    // Make sure the pager is not enabled.
    if ($this->display->handler->use_pager()) {
      $errors[] = t('The Juicebox style cannot be used with a pager. Please disable the "Use pager" option for this display.');
    }
    // We want to somewhat "nag" the user if they have not yet configured the
    // Juicebox-specific plugin settings (because things won't work until they
    // do). However, we do NOT want to formally set an error. This is because
    // this validate method can run on pages where the user can't actaully touch
    // the Juicebox-specific plugin settings (such as
    // admin/structure/views/add).
    static $nag_message_set = FALSE;
    if (!$nag_message_set && empty($this->options['image_field'])) {
      drupal_set_message(t("To ensure a fully functional Juicebox gallery please remember to add at least one image field to your Juicebox view display and configure the Juicebox Gallery format settings."), 'warning');
      $nag_message_set = TRUE;
    }
    return $errors;
  }

  /**
   * Render the view page display.
   *
   * This is where the Juicebox embed code is built for the view.
   */
  function render() {
    $view = $this->view;
    // If we are previewing the view in the admin interface all the necessary
    // <script> elements in our embed markup seem to get stripped. This means
    // that the preview cannot render and so there is no point in loading the
    // Juicebox lib (doing so can actually cause display problems).
    if (strpos(current_path(), 'admin/structure/views') !== 0) {
      // Load the juicebox library, this will include the appropriate js in the
      // page header, etc.
      libraries_load('juicebox');
    }
    else {
      // Post a notice to admins so that they don't think things are broken
      // when the preview does not produce anything useful.
      drupal_set_message(t("Due to javascript requirements Juicebox galleries cannot be viewed as a live preview. Please save your view and visit the full page URL for this display to preview this gallery."), 'warning');
    }
    $settings = $view->style_plugin->options;
    // Generate a unique ID that can be used to identify this gallery and view
    // source details.
    $xml_id = 'view/' . $view->name . '/' . $view->current_display;
    foreach ($view->args as $arg) {
      $xml_id .= '/' . $arg;
    }
    $xml_path = 'juicebox/xml/' . $xml_id;
    $xml_args = explode('/', $xml_path);
    
    // Instantiate and build the Juicebox gallery.
    try {
      $juicebox = juicebox($xml_args, $view);
      // Build the gallery.
      $juicebox->build();
      $element = array('#markup' => $juicebox->renderEmbed($xml_path, $settings));
    }
    catch (Exception $e) {
      $message = 'Exception building Juicebox embed code: !message in %function (line %line of %file).';
      watchdog_exception('juicebox', $e, $message);
    }
    return $element;
  }
}
