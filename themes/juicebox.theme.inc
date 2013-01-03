<?php

/**
 * @file
 * Theme related functions for processing our output style plugins.
 *
 * Views bug: http://drupal.org/node/593336
 */

// Views templage preprocess function for our juicebox style view. This function
// renders the Juicebox XML, caches it, and then sets the variabels needed to
// generate the Juicebox embed markup (which is what this view displays).
function template_preprocess_juicebox_view(&$variables) {
  global $base_url;
  $view = $variables['view'];
  // Render the Juicebox "confix.xml" for this view
  $xml = juicebox_render_xml_from_view($view);
  // Generate a unique view ID that can be used to label the XML in the cache
  $view_id = $view->name . '_' . $view->current_display;
  foreach ($view->args as $arg) {
    $view_id .= '_' . $arg;
  }
  // Cache the xml in the Drupal "cache_page" table. This will be fethched
  // via a separate URL witin the Juicebox embed markup. Our cached value will
  // refresh itself each time the view is rendered, but we still set an expire 
  // time (one day) just so that stale entries will not linger. 
  cache_set('juicebox_xml-' . $view_id, $xml, 'cache_page', time() + 86400);
  // Set some variables that our template file will use when rendering the
  // final Jucebox ember markup
  $variables['view_id'] = $view_id;
  $variables['juicebox_path'] = '';
  $juicebox_library = libraries_detect('juicebox');
  if ($juicebox_library['installed']) {
    $variables['juicebox_path'] = $juicebox_library['library path'];
  }
  else {
    watchdog('juicebox', 'The Juicebox library does not appear to be installed correctly.', array(), WATCHDOG_ERROR);
  }
  $variables['config_url_path'] = $base_url . '/juicebox/xml/view/' . $view_id;
  $variables['style_options'] = $view->style_options;
}

