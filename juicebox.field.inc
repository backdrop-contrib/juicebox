<?php


/**
 * @file
 * Contains all hooks and related methods for the juicebox_formatter field
 * formatter.
 */


/**
 * Implements hook_field_formatter_info().
 *
 * Add juicebox_formatter formatter.
 */
function juicebox_field_formatter_info() {
  // Get the base settings. This function may be called while clearing caches
  // so let's be extra careful about catching any errors gracefully.
  try {
    $base_settings = juicebox()->confBaseOptions();
  }
  catch (Exception $e) {
    $message = 'Exception fetching base configuration options for Juicebox field formatter: !message in %function (line %line of %file).';
    watchdog_exception('juicebox', $e, $message);
    return array();
  }
  $formatters = array(
    'juicebox_formatter' => array(
      'label' => t('Juicebox Gallery'),
      'description' => t('Style groups of images or files as a Juicebox gallery.'),
      'field types' => array('image', 'file'),
      'settings' => array_merge($base_settings, array(
        'image_style' => '',
        'thumb_style' => 'juicebox_square_thumbnail',
        'caption_source' => 'alt',
        'title_source' => 'title',
      )),
    ),
  );
  return $formatters;
}


/**
 * Implements hook_field_formatter_settings_form().
 */
function juicebox_field_formatter_settings_form($field, $instance, $view_mode, $form, &$form_state) {
  // If this formatter is being used outside the context of a normal field
  // display (e.g., within individual view rows, panels, as a file entity
  // display, etc.) we display a message to discourage and prevent it's use.
  // This is due to the fact that we don't have a reliable way to build the XML
  // for these cases.
  // @todo Consider a "whitelisting" approach instead of "blacklisting".
  $juicebox_formatter_notice = '';
  if (!empty($instance['bundle']) && $instance['bundle'] == 'ctools') {
    $juicebox_formatter_notice = t('<strong>NOTICE: The Juicebox field formatter is not fully supported within individual view rows (and other ctools contexts)</strong>. Using this setting is not recommended. For views support please use the Juicebox display style plugin.');
  }
  if (!empty($instance['entity_type']) && $instance['entity_type'] == 'file') {
    $juicebox_formatter_notice = t('<strong>NOTICE: The Juicebox field formatter is not fully supported when rendering individual file entities</strong>. Using this setting is not recommended. For file support please apply the Juicebox field formatter to a <em>field</em> of type file.');
  }
  if (!empty($juicebox_formatter_notice)) {
    return array('juicebox_formatter_notice' => array(
      '#prefix' => '<p>',
      '#markup' => $juicebox_formatter_notice,
      '#suffix' => '</p>',
    ));
  }
  $form = array();
  // Get available title and caption sources.
  $text_sources = _juicebox_field_text_sources($instance);
  // Get available image style presets
  $presets = image_style_options(FALSE);
  $settings = $instance['display'][$view_mode]['settings'];
  // Add the field-formatter-specific elements.
  $form['image_style'] = array(
    '#type' => 'select',
    '#title' => t('Main Image Style'),
    '#default_value' => $settings['image_style'],
    '#description' => t('The style formatter for the main image.'),
    '#options' => $presets,
    '#empty_option' => t('None (original image)'),
  );
  $form['thumb_style'] = array(
    '#type' => 'select',
    '#title' => t('Thumbnail Style'),
    '#default_value' => $settings['thumb_style'],
    '#description' => t('The style formatter for the thumbnail.'),
    '#options' => $presets,
    '#empty_option' => t('None (original image)'),
  );
  $form['caption_source'] = array(
    '#type' => 'select',
    '#title' => t('Caption Source'),
    '#default_value' => $settings['caption_source'],
    '#description' => t('The image value that should be used for the caption.'),
    '#options' => $text_sources,
    '#empty_option' => t('No caption'),
  );
  $form['title_source'] = array(
    '#type' => 'select',
    '#title' => t('Title Source'),
    '#default_value' => $settings['title_source'],
    '#description' => t('The image value that should be used for the title.'),
    '#options' => $text_sources,
    '#empty_option' => t('No title'),
  );
  // Add common Juicebox form options.
  $form = juicebox()->confBaseForm($form, $settings);
  return $form;
}


/**
 * Implements hook_field_formatter_settings_summary().
 */
function juicebox_field_formatter_settings_summary($field, $instance, $view_mode) {
  $display = $instance['display'][$view_mode];
  $settings = $display['settings'];
  // Get available image style presets
  $presets = image_style_options(FALSE);
  $settings_display = array();
  // Image style setting.
  if (!empty($settings['image_style']) && isset($presets[$settings['image_style']])) {
    $style = $presets[$settings['image_style']];
  }
  else {
    $style = t('Original Image');
  }
  $settings_display[] = t("Image style: @style", array('@style' => $style));
  // Thumb style setting.
  if (!empty($settings['thumb_style']) && isset($presets[$settings['thumb_style']])) {
    $style = $presets[$settings['thumb_style']];
  }
  else {
    $style = t('Original Image');
  }
  $settings_display[] = t("Thumbnail style: @style", array('@style' => $style));
  // Define display options for caption and title source.
  $text_sources = _juicebox_field_text_sources($instance);
  // Caption source setting.
  if (!empty($text_sources[$settings['caption_source']])) {
    $source = $text_sources[$settings['caption_source']];
  }
  else {
    $source = t('None');
  }
  $settings_display[] = t("Caption source: @source", array('@source' => $source));
  // Title source setting.
  if (!empty($text_sources[$settings['title_source']])) {
    $source = $text_sources[$settings['title_source']];
  }
  else {
    $source = t('None');
  }
  $settings_display[] = t("Title source: @source", array('@source' => $source));
  // Add-in a note about the additional fieldsets.
  $settings_display[] = t("Additional Juicebox library configuration options may also be set.");
  $summary = implode('<br />', $settings_display);
  return $summary;
}


/**
 * Implements hook_field_formatter_view().
 */
function juicebox_field_formatter_view($entity_type, $entity, $field, $instance, $langcode, $items, $display) {
  $element = array();
  // If there are no images, don't do anything else. Also, for some reason
  // field-level access is not checked until AFTER this hook, so we run our own
  // access check here to ensure no gallery is rendered if the embed code won't
  // be shown (preventing the javascript from being unnecessarily included).
  if (empty($items) || !field_access('view', $field, $entity_type, $entity)) {
    return $element;
  }
  // The gallery shown in preview view will only display field data from the
  // previously saved version (that is the only version the XML generation
  // methods will have access to). Display a warning because of this.
  if (!empty($entity->in_preview)) {
    drupal_set_message(t('Juicebox galleries may not display correctly in preview mode. Any edits made to gallery data will only be visible after all changes are saved.'), 'warning', FALSE);
  }
  $field_name = $instance['field_name'];
  $entity_type_info = entity_get_info($entity_type);
  $entity_id = $entity->{$entity_type_info['entity keys']['id']};
  // We need to get the display name to pass as part of our XML path. Though
  // we have access to the actaul $display array, it does not look like we
  // have access to the actaul display NAME in this scope. We do have access to
  // a list of ALL displays in $instanace though, so iterate though them to
  // find a match to the settings in $display.
  $display_name = 'default';
  foreach ($instance['display'] as $display_key => $display_data) {
    if ($display['settings'] == $display_data['settings']) {
      $display_name = $display_key;
    }
  }
  // Generate xml path details.
  $xml_id = 'field/' . $entity_type . '/' . $entity_id . '/' . $field_name . '/' . $display_name;
  $xml_args = explode('/', $xml_id);
  $xml_path = 'juicebox/xml/' . $xml_id;
  // Try building the gallery and its XML.
  try {
    // Initialize the gallery.
    $juicebox = juicebox();
    $juicebox->init($xml_args, $display['settings'], $items);
    // Build the gallery.
    juicebox_field_build_gallery($juicebox, $items);
    // Create a render array with the gallery markup. Also include the built
    // gallery for external reference.
    $element[0] = array('#markup' => $juicebox->renderEmbed($xml_path), '#juicebox' => $juicebox);
  }
  catch (Exception $e) {
    $message = 'Exception building Juicebox embed code for field: !message in %function (line %line of %file).';
    watchdog_exception('juicebox', $e, $message);
  }
  return $element;
}


/**
 * Utility to build a Juicebox gallery based on field formatter data.
 *
 * @param JuiceboxGalleryWrapperInterface $juicebox
 *   A juicebox object ready to be manipulated into a gallery.
 * @param array $items
 *   An array for file field items that have been loaded to represent individual
 *   gallery images.
 */
function juicebox_field_build_gallery(JuiceboxGalleryDrupalInterface $juicebox, $items) {
  // Initialize the gallery.
  $settings = $juicebox->getSettings();
  foreach ($items as $id => $item) {
    // Calculate the source data that Juicebox requires.
    $src_data = $juicebox->styleImageSrcData($item, $settings['image_style'], $item, $settings['thumb_style']);
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
      $title = _juicebox_field_get_text($item, $settings['title_source']);
    }
    // Set the image caption.
    $caption = _juicebox_field_get_text($item, $settings['caption_source']);
    // Add this image to the gallery.
    $juicebox->addImage($src_data, $title, $caption);
  }
  // Run common build tasks.
  $juicebox->runCommonBuild();
}


/**
 * Utility to get sanitized text directly from a file field item.
 *
 * This method will attempt to extract text, in a format safe for display,
 * from the data contained within a file field item.
 *
 * @param array $item
 *   An associative array representing a file field item.
 * @param string $source
 *   The key within the $item array that contains the text that we want to
 *   extract.
 * @return string
 *   Safe text for output or an empty string if no text can be extracted.
 *
 * @see _juicebox_field_text_sources()
 */
function _juicebox_field_get_text($item, $source) {
  $text = '';
  if (!empty($item[$source])) {
    // If a field option is specified we need to try to get the value from a
    // file entity. Note that the entity can be re-created by casting the
    // field item into an object.
    if (strpos($source, 'field_') === 0) {
      $field = field_get_items('file', (object) $item, $source);
      if (!empty($field[0]['safe_value'])) {
        $text = $field[0]['safe_value'];
      }
    }
    // If an image field caption value is specified we need to format the raw
    // caption value.
    elseif ($source == 'image_field_caption' && !empty($item['image_field_caption']['value'])) {
      $format = empty($item['image_field_caption']['format']) ? filter_fallback_format() : $item['image_field_caption']['format'];
      $text = check_markup($item['image_field_caption']['value'], $format);
    }
    else {
      $text = check_markup($item[$source]);
    }
  }
  return $text;
}


/**
 * Utility to fetch the title and caption source options for field-based
 * galleries (addresses File Entity and Media module support).
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
 * @see _juicebox_field_get_text()
 */
function _juicebox_field_text_sources($instance) {
  // The filename should always be an available option.
  $text_source_options['filename'] = t('File - Filename (processed by fallback text format)');
  if (!empty($instance['widget']['module'])) {
    // If this is a standard image field, using the image widget, we can use
    // core image "alt" and "title" values if they are active.
    if ($instance['widget']['module'] == 'image') {
      if (!empty($instance['settings']['alt_field'])) {
        $text_source_options['alt'] = t('Image - Alt text (processed by fallback text format)');
      }
      if (!empty($instance['settings']['title_field'])) {
        $text_source_options['title'] = t('Image - Title text (processed by fallback text format)');
      }
      // If image field caption is installed and enabled on this image field
      // we can also use it.
      if (module_exists('image_field_caption') && !empty($instance['settings']['image_field_caption'])) {
        $text_source_options['image_field_caption'] = t('Image - Image field caption');
      }
    }
    // If this is a standard file field, using the file widget, we can use the
    // core file "description" value if it is active.
    if ($instance['widget']['module'] == 'file' && !empty($instance['settings']['description_field'])) {
      $text_source_options['description'] = t('File - Description text (processed by fallback text format)');
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
