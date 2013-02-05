<?php

/**
 * @file
 * Theme related functions for processing output related to Juicebox galleries.
 */


/**
 * Returns HTML for embedding a Juicebox gallery.
 *
 * @param array $variables
 *   An associative array containing variables used to render this HTML.
 *
 * @ingroup themeable
*/
function theme_juicebox_embed_markup(&$variables) {
  // Create a gallery id from the XML path
  $gallery_id = trim($variables['config_url_path'], '/');
  $gallery_id = str_replace(array('/', ':'), '-', $gallery_id);
  $output = "<script>" . "\n";
  $output .= "  new juicebox({" . "\n";
  $output .= "    configUrl : '" . $variables['config_url_path'] . "'," . "\n";
  $output .= "    containerId : '" . $gallery_id . "'," . "\n";
  $output .= "    galleryWidth : '" . check_plain($variables['style_options']['width']) . "'," . "\n";
  $output .= "    galleryHeight : '" . check_plain($variables['style_options']['height']) . "'" . "\n";
  $output .= "  });" . "\n";
  $output .= "</script>" . "\n";
  $custom_classes = '';
  if (!empty($variables['style_options']['advanced']['custom_parent_classes'])) {
    $custom_classes = ' ' . check_plain(trim($variables['style_options']['advanced']['custom_parent_classes']));
  }
  $output .= '<div class="juicebox-parent' . $custom_classes . '">' . "\n";
  $output .= '<div id="' . $gallery_id . '" class="juicebox-container"></div>' . "\n";
  $output .= '</div>' . "\n";
  return $output;
}


/**
 * Returns the XML used to describe a Juicebox gallery.
 *
 * @param array $variables
 *   An associative array containing variables used to render this XML.
 *
 * @ingroup themeable
*/
function theme_juicebox_config_xml($variables) {
  $output = '';
  $output = '<?xml version="1.0" encoding="UTF-8"?>' . "\n";
  $output .= '<juiceboxgallery' . "\n";
  foreach ($variables['options'] as $option) {
    $output .= '  ' . trim($option) . "\n";
  }
  $output .= '>' . "\n";
  // Add each image to the xml
  foreach ($variables['images'] as $image) {
    $output .= '  <image imageURL="' . $image['image_src'] . '"' . "\n";
    $output .= '    thumbURL="' . $image['thumb_src'] . '"' . "\n";
    $output .= '    linkURL="' . $image['image_src'] . '"' . "\n";
    $output .= '    linkTarget="_blank">' . "\n";
    $output .= '    <title>' . $image['title'] . '</title>' . "\n";
    $output .= '    <caption><![CDATA[' . $image['caption'] . ']]></caption>' . "\n";
    $output .= '  </image>' . "\n";
  }
  $output .= '</juiceboxgallery>';
  return $output;
}


/**
 * Returns HTML for embedding a Juicebox gallery.
 * 
 * The views style plugin requires its own theme function and this one is really
 * just a wrapper for the juicebox_embed_markup theme call at the end.
 * 
 * @see: theme_juicebox_embed_markup()
 *
 * @param array $variables
 *   An associative array containing variables used to render this HTML.
 *
 * @ingroup themeable
*/
function theme_juicebox_view(&$variables) {
  global $base_url;
  $view = $variables['view'];
  // Generate a unique ID that can be used to identify this view when
  // fetching the XML
  $xml_id = '/view/' . $view->name . '/' . $view->current_display;
  foreach ($view->args as $arg) {
    $xml_id .= '/' . $arg;
  }
  // If we are previewing the view in the admin interface all the necessary
  // <script> elements in our embed markup seem to get stripped. This means
  // that the preview cannot render and thus there is no point in loading the
  // Juicebox lib (doing so can actually cause display problems).
  $args = arg();
  $path = '';
  foreach ($args as $arg) {
    $path .= '/' . $arg;
  }
  if (strpos($path, '/admin/structure/views/') !== 0) {
    // Load the juicebox library, this will include the appropriate js in the
    // page header, etc.
    libraries_load('juicebox');
  }
  else {
    // Post a notice to admins so that they don't think things are broken
    // when the preview does not produce anything useful.
    drupal_set_message(t("Due to javascript requirements Juicebox galleries cannot be viewed as a live preview. Please save your view and visit the full page URL for this display to preview this gallery."), 'warning');
  }
  // Set template variables for embed markup.
  $variables['config_url_path'] = url('juicebox/xml' . $xml_id);
  $variables['style_options'] = $view->style_options;
  return theme('juicebox_embed_markup', $variables);
}
