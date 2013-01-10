<?php

/**
 * @file
 * Theme related functions for processing our output style plugins.
 */


// Theme function for our juicebox view style. The views style plugin requires
// its own theme function and this one is really just a wrapper for the 
// juicebox_embed_markup theme call at the end.
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


// Theme the common juicebox embed markup
function theme_juicebox_embed_markup(&$variables) {
  $output = "<script>" . "\n";
  $output .= "  new juicebox({" . "\n";
  $output .= "    configUrl : '" . $variables['config_url_path'] . "'," . "\n";
  $output .= "    containerId : 'juicebox-container'," . "\n";
  $output .= "    galleryWidth : '" . check_plain($variables['style_options']['width']) . "'," . "\n";
  $output .= "    galleryHeight : '" . check_plain($variables['style_options']['height']) . "'" . "\n";
  $output .= "  });" . "\n";
  $output .= "</script>" . "\n";
  $output .= '<div id="juicebox-container"></div>' . "\n";
  return $output;
}


// Theme an array of XML variable data into a Juicebox XML format.
function theme_juicebox_config_xml($variables) {
  $output = '';
  $output = '<?xml version="1.0" encoding="UTF-8"?>' . "\n";
  $output .= '<juiceboxgallery' . "\n";
  foreach ($variables['options'] as $option) {
    $output .= '  ' . $option . "\n";
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
