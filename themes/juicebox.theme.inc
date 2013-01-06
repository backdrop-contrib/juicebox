<?php

/**
 * @file
 * Theme related functions for processing our output style plugins.
 */


// Views template preprocess function for our juicebox style view. This function
// renders the Juicebox XML, caches it, and then sets the variables needed to
// generate the Juicebox embed markup (which is what this view displays).
function template_preprocess_juicebox_view(&$variables) {
  global $base_url;
  $view = $variables['view'];
  // Generate a unique view ID that can be used to label the XML in the cache
  $cid = '/view/' . $view->name . '/' . $view->current_display;
  foreach ($view->args as $arg) {
    $view_id .= '/' . $arg;
  }
  // Cache the xml in our "cache_juicebox" table. This XML will be fetched
  // via a separate URL within the Juicebox embed markup, so it only really
  // needs to sit in the cache long enough for the embed html to be rendered.
  // Our cached value will refresh itself each time the embed code is rendered
  // so the expiration time should not really matter, but to be safe we set
  // it to match the core system cache lifetime.
  $xml = juicebox_get_xml_from_view($view);
  cache_set($cid, $xml, 'cache_juicebox', time() + variable_get('cache_lifetime', 0));
  // If we are previewing the view in the admin interface all the necessary
  // <script> elements in our display template seem to get stripped. This means
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
    $juicebox_library = libraries_load('juicebox');
  }
  else {
    // Post a notice to admins so that they don't think things are broken
    // when the preview does not produce anything useful.
    drupal_set_message(t("Due to javascript requirements Juicebox galleries cannot be viewed as a live preview. Please save your view and visit the full page URL for this display to preview this gallery."), 'warning');
  }
  // Set template variables for embed markup.
  $variables['config_url_path'] = $base_url . '/juicebox/xml' . $cid;
  $variables['style_options'] = $view->style_options;
}


// Theme an array of XML variable data into a Juicebox XML format. This is done
// as a Drupal theme function not because we expect a need for this to be
// overridden, but because it provides a drupaly way to consolidate all the
// "rendering" of the final XML within a common function.
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